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

VectorXd B;

void Field::initialize(){

  srand(time(NULL));
  SEED = 100;//rand();
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

  //Simple Double Counter-Current Flow
  double k;
  k = (SEED % 2 == 0)?1.0:-1.0;
  b1 += (k*shape::flatGaussian(glm::vec2(0.0, SIZE/2.0), SIZE/2.0)).matrix();
  b1 += (k*shape::flatGaussian(glm::vec2(SIZE, SIZE/2.0), SIZE/2.0)).matrix();
  //Only half as fast at the top...
  b2 -= (k*0.2*shape::flatGaussian(glm::vec2(SIZE/2.0, 0.0), SIZE/2.0)).matrix();
  b2 -= (k*0.2*shape::flatGaussian(glm::vec2(SIZE/2.0, SIZE), SIZE/2.0)).matrix();

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

  B = b1 + b2;

  //Important!
  PDE::initialize(dx, dy);
}

VectorXd HSOURCE;
VectorXd TSOURCE;

void Field::timestep(){

  //Rotate Blower
  int time = 365;
  if(count++ >= time) count = 0;
  float t = 2.0*3.14159265*(count%time)/(double)time;
  v0 = glm::vec2(sin(t), cos(t));

  //Solve the Navier Stokes Equations
  PDE::navierstokes(dt, viscosity, vX, vY, P, bc, g, v0, B);

  //Transport Species
  HSOURCE = source::HSOURCE(bc, height, sealevel, humidity, P, temperature);
  PDE::transport(dt, diffusivity, humidity, PDE::VMAT, hc, HSOURCE);
  humidity = humidity.cwiseMax(0.0);

  TSOURCE = source::TSOURCE(bc, height, sealevel, humidity, P, temperature, vX, vY);
  PDE::transport(dt, conductivity, temperature, PDE::VMAT, hc, TSOURCE);
  temperature = temperature.cwiseMax(0.0);

}
