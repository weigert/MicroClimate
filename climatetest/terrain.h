#include <noise/noise.h>

using namespace Eigen;

class Field{
public:
  //Grid-Size
  double dx = 1.0/(double)SIZE;
  double dy = 1.0/(double)SIZE;
  double dt = 0.025;

  //Parameters
  double viscosity = 0.001;    //[m^2/s]  //Future: Replace with Temperature Dependent?
  double density = 1.225;       //[kg/m^3]Future: Replace with Temperature Dependent?
  double P0 = 1E+5;             //Initial Pressure [Pa]

  int SEED = 0;

  //Volume Force
  glm::vec2 g = glm::vec2(1.0, 1.0);
  glm::vec2 v0 = glm::vec2(1.0, 0.0); //Terminal Velocity at Driving Force

  float sealevel;

  //Integration Stuff
  void initialize();
  void timestep();
  int count = 0;
  bool divergence = false;
  double err = 100.0; //Initial Mass Error

  //Velocities (Actually)
  VectorXd vX;
  VectorXd vY;
  VectorXd P;

  VectorXd height;
  VectorXd bc;

  //Extra Fields
  VectorXd humidity;
  VectorXd temperature;
  double H0 = 0.01;  //Initial Humidity
  double T0 = 25.0; //Celsius

  //If these two guys are too large, the disturbances propagate like a wave
  //i.e. elliptically instead of characteristically
  //If their ratio is too different, then one propagates too fast or too slow
  //Meaning you get these huge eliminations or no spread of rain nucleation
  double diffusivity = 0.0004;
  double conductivity = 0.0008; //Important: Diffusivity Lower than Conductivity (Slightly)
};

VectorXd hc = ArrayXd::Zero(SIZE*SIZE);
VectorXd b1 = ArrayXd::Zero(SIZE*SIZE);
VectorXd b2 = ArrayXd::Zero(SIZE*SIZE);

SparseMatrix<double> DMAT;


void Field::initialize(){
  srand(time(NULL));
  SEED = rand();
  std::cout<<SEED<<std::endl;

  //Height Map Generation
  noise::module::Perlin perlin;
  perlin.SetOctaveCount(10);
  perlin.SetFrequency(6);
  perlin.SetPersistence(0.5);
  height = ArrayXd::Zero(SIZE*SIZE);
  for(int i = 0; i < SIZE*SIZE; i++){
    glm::vec2 pos = alg::pos(i);
    height(i) = perlin.GetValue(pos.x/(double)SIZE, pos.y/(double)SIZE, SEED);
  }

  VectorXd E = ArrayXd::Ones(SIZE*SIZE);

  //Rescale the Height (First Multiply by Gaussian)
  height = (height.array()*shape::flatGaussian(glm::vec2(SIZE/2.0, SIZE/2.0), 5.0*SIZE)).matrix();
  double hmin = height.minCoeff();
  double hmax = height.maxCoeff();
  height = (height - hmin*E)/(hmax - hmin);
  sealevel = 1.05*height.mean();

  //Compute the Boundary
  bc = ArrayXd::Zero(SIZE*SIZE);
  for(int i = 0; i < SIZE*SIZE; i++){
    //We can raise the boundary by altering this ratio...
    hc(i) = (1.0-height(i))/(1.0-sealevel)*0.9;
    bc(i) = (height(i) > sealevel)?hc(i)*0.3:1.0;
  }

  //Square it for more emphasis on choking effect
  bc = bc.cwiseProduct(bc);

  //Blower Intensity...

  /*
  //Alternative: Side-Driven
  b1 += shape::flatGaussian(glm::vec2(0.0, SIZE/2.0), SIZE/2.0);
  b2 += shape::flatGaussian(glm::vec2(SIZE, SIZE/2.0), SIZE/2.0);
  */

  //Simple Double Counter-Current Flow
  double k;
  k = (SEED % 2 == 0)?1.0:-1.0;
  b1 += (k*shape::flatGaussian(glm::vec2(0.0, SIZE/2.0), SIZE/2.0)).matrix();
  b1 += (k*shape::flatGaussian(glm::vec2(SIZE, SIZE/2.0), SIZE/2.0)).matrix();
  //Only half as fast at the top...
  b2 -= (k*0.2*shape::flatGaussian(glm::vec2(SIZE/2.0, 0.0), SIZE/2.0)).matrix();
  b2 -= (k*0.2*shape::flatGaussian(glm::vec2(SIZE/2.0, SIZE), SIZE/2.0)).matrix();

  //Counter Flow
  /*
  b1 += shape::flatGaussian(glm::vec2(SIZE/2.0, 0.0), SIZE/2.0);
  b1 += shape::flatGaussian(glm::vec2(SIZE/2.0, SIZE), SIZE/2.0);

  b1 -= shape::flatGaussian(glm::vec2(0.0, SIZE/4.0), SIZE/2.0);
  b1 -= shape::flatGaussian(glm::vec2(SIZE, SIZE/4.0), SIZE/2.0);

  b1 += shape::flatGaussian(glm::vec2(0.0, SIZE/2.0), SIZE/2.0);
  b1 += shape::flatGaussian(glm::vec2(SIZE, SIZE/2.0), SIZE/2.0);

  b1 -= shape::flatGaussian(glm::vec2(0.0, 3.0*SIZE/4.0), SIZE/2.0);
  b1 -= shape::flatGaussian(glm::vec2(SIZE, 3.0*3SIZE/4.0), SIZE/2.0);
*/

  //Set humidity
  humidity = 0.0*E;
  temperature = T0*E;

  //Initialize Pressure to some pressures pike in the center...
  P = P0*E;

  //Initialize Velocities to Zero
  vX = ArrayXd::Zero(SIZE*SIZE);
  vY = ArrayXd::Zero(SIZE*SIZE);
  vX = bc.asDiagonal()*vX;
  vY = bc.asDiagonal()*vY;

  PDE::initialize(dx, dy);

  //General Operators
  DMAT = PDE::XDIFFUSION + PDE::YDIFFUSION;
}

SparseMatrix<double> E_MAT;
VectorXd P_SOURCE_X;
VectorXd P_SOURCE_Y;
SparseMatrix<double> I_MAT;
VectorXd A;
SparseMatrix<double> TRANSPORTMAT_H;
SparseMatrix<double> TRANSPORTMAT_T;
VectorXd HSOURCE;
VectorXd TSOURCE;

SparseMatrix<double> SE = alg::sparseIdentity();
SparseMatrix<double> VMAT;

VectorXd vXG;
VectorXd vYG;

VectorXd dP;
VectorXd dvX;
VectorXd dvY;

void Field::timestep(){
  int time = 365;
  if(count++ >= time) count = 0;
  v0 = glm::vec2(1.0, sin(2.0*3.14159265*(count%time)/(double)time));

  VMAT = -1.0*(vX.asDiagonal()*PDE::XFLUX);
  VMAT -= vY.asDiagonal()*PDE::YFLUX;

  //Construct Non-Linear Operator from Velocity Field and Other Contributions
  E_MAT = bc.asDiagonal()*(VMAT + viscosity*DMAT);

  P_SOURCE_X = bc.cwiseProduct(g.x*(b1+b2).cwiseProduct(VectorXd::Ones(SIZE*SIZE)*v0.x-vX));
  P_SOURCE_Y = bc.cwiseProduct(g.y*(b1+b2).cwiseProduct(VectorXd::Ones(SIZE*SIZE)*v0.y-vY));
  P_SOURCE_X.noalias() -= bc.cwiseProduct(PDE::XFLUX*P);
  P_SOURCE_Y.noalias() -= bc.cwiseProduct(PDE::YFLUX*P);

  //Helper Values (Sum over Rows)
  I_MAT = SE - dt*E_MAT;
  A = I_MAT*VectorXd::Ones(SIZE*SIZE);

  //Crank Nicholson
  vX.noalias() += 0.5*dt*(E_MAT*vX);
  vX.noalias() += 0.5*dt*P_SOURCE_X;
  PDE::solver.compute(SE-0.5*dt*E_MAT);
  vX = PDE::solver.solve(vX + 0.5*dt*P_SOURCE_X);

  vY.noalias() += 0.5*dt*(E_MAT*vY);
  vY.noalias() += 0.5*dt*P_SOURCE_Y;
  PDE::solver.factorize(SE-0.5*dt*E_MAT);
  vY = PDE::solver.solve(vY + 0.5*dt*P_SOURCE_Y);

  //Expressions Optimized for Eigen
  TRANSPORTMAT_H = hc.asDiagonal()*VMAT;
  TRANSPORTMAT_T = TRANSPORTMAT_H;
  TRANSPORTMAT_H += diffusivity*hc.asDiagonal()*DMAT;
  TRANSPORTMAT_T += conductivity*hc.asDiagonal()*DMAT;
  HSOURCE = source::HSOURCE(bc, height, sealevel, humidity, P, temperature);
  TSOURCE = source::TSOURCE(bc, height, sealevel, humidity, P, temperature, vX, vY);

  //Crank Nicholson
  humidity.noalias() += 0.5*dt*(TRANSPORTMAT_H*humidity);
  humidity.noalias() += 0.5*dt*HSOURCE;
  PDE::solver.compute(SE-0.5*dt*TRANSPORTMAT_H);
  humidity = PDE::solver.solve(humidity + 0.5*dt*HSOURCE);

  temperature.noalias() += 0.5*dt*(TRANSPORTMAT_T*temperature);
  temperature.noalias() += 0.5*dt*TSOURCE;
  PDE::solver.factorize(SE-0.5*dt*TRANSPORTMAT_T);
  temperature = PDE::solver.solve(temperature + 0.5*dt*TSOURCE);

  double newerr = 1.0;
  double pCorr = 1.0;
  int maxiter = 250;
  int n = 0;

  while(newerr > 1E-4 && pCorr > 1E-6 && !divergence && maxiter){
    n++;

    VMAT = -1.0*vX.asDiagonal()*PDE::XFLUX;
    VMAT -= vY.asDiagonal()*PDE::YFLUX;
    E_MAT = bc.asDiagonal()*(VMAT + viscosity*DMAT);

    //Compute the Intermediary Values
    P_SOURCE_X = vX;
    P_SOURCE_Y = vY;
    P_SOURCE_X.noalias() += dt*bc.cwiseProduct(g.x*(b1+b2).cwiseProduct(VectorXd::Ones(SIZE*SIZE)*v0.x-vX));
    P_SOURCE_Y.noalias() += dt*bc.cwiseProduct(g.y*(b1+b2).cwiseProduct(VectorXd::Ones(SIZE*SIZE)*v0.y-vY));
    P_SOURCE_X.noalias() -= dt*bc.cwiseProduct(PDE::XFLUX*P);
    P_SOURCE_Y.noalias() -= dt*bc.cwiseProduct(PDE::YFLUX*P);

    //E_MAT is updated already...
    I_MAT = SE - dt*E_MAT;
    A = I_MAT*VectorXd::Ones(SIZE*SIZE);

    //Pressure Correction
    PDE::solver.compute(I_MAT);
    vXG = PDE::solver.solve(P_SOURCE_X);
    vYG = PDE::solver.solve(P_SOURCE_Y);

    //Pressure Correction
    PDE::solver.compute((PDE::GXF*A.cwiseInverse().asDiagonal()*PDE::GXB+ PDE::GYF*A.cwiseInverse().asDiagonal()*PDE::GYB)); //2D Laplace Operator
    dP = PDE::solver.solve((PDE::GXB*vXG + PDE::GYB*vYG));

    // We compute the velocity correction based on the pressure correction
    dvX = -(PDE::GXF*dP).cwiseQuotient(A);
    dvY = -(PDE::GYF*dP).cwiseQuotient(A);

    // We correct our velocity guesses from the intermediary field
    vX = vXG;
    vY = vYG;
    vX.noalias() += 0.9*bc.cwiseProduct(dvX);
    vY.noalias() += 0.9*bc.cwiseProduct(dvY);

    // We correct our pressure guess from our previous guess
    P.noalias() += bc.cwiseProduct(dP);

    // Compute the Error (Divergence of Field) simply using our guess.
    newerr = (PDE::GXB*vXG + PDE::GYB*vYG + dvY + dvX).squaredNorm()/(SIZE*SIZE);
    pCorr = dP.squaredNorm()/(SIZE*SIZE);

    if(newerr > err) divergence = true;
    maxiter--;

    TRANSPORTMAT_H = hc.asDiagonal()*(VMAT);
    TRANSPORTMAT_T = hc.asDiagonal()*(VMAT);
    TRANSPORTMAT_H += diffusivity*hc.asDiagonal()*(DMAT);
    TRANSPORTMAT_T += conductivity*hc.asDiagonal()*(DMAT);
    HSOURCE = source::HSOURCE(bc, height, sealevel, humidity, P, temperature);
    TSOURCE = source::TSOURCE(bc, height, sealevel, humidity, P, temperature, vX, vY);

    //Crank Nicholson
    humidity.noalias() += 0.5*dt*(TRANSPORTMAT_H*humidity);
    humidity.noalias() += 0.5*dt*HSOURCE;
    PDE::solver.compute(SE-0.5*dt*TRANSPORTMAT_H);
    humidity = PDE::solver.solve(humidity + 0.5*dt*HSOURCE);

    temperature.noalias() += 0.5*dt*(TRANSPORTMAT_T*temperature);
    temperature.noalias() += 0.5*dt*TSOURCE;
    PDE::solver.factorize(SE-0.5*dt*TRANSPORTMAT_T);
    temperature = PDE::solver.solve(temperature + 0.5*dt*TSOURCE);

    humidity = humidity.cwiseMax(0.0);
  }
}
