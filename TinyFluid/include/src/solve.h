//Full Transport Solvers
namespace PDE{
using namespace Eigen;

typedef SparseMatrix<double,RowMajor> smat;

smat GXF;       //Gradient Operator X-Element
smat GYF;       //Gradient Operator Y-Element
smat GXB;       //Gradient Operator X-Element
smat GYB;       //Gradient Operator Y-Element
smat XFLUX;     //Integrated Surface Flux X, UNIFORM AREA!!
smat YFLUX;

Eigen::VectorXd E;

smat DIFFUSIONMAT;
smat TRANSPORTMAT;

void initialize(double dx, double dy){
  E = ArrayXd::Ones(SIZE*SIZE);

  //Discrete Differential Operators (of some order)
  GXF = space::FFD(glm::vec2(1, 0), 1)/dx;  //Gradient needs to be divided by the grid-spacing
  GYF = space::FFD(glm::vec2(0, 1), 1)/dy;  //-``-
  GXB = space::BFD(glm::vec2(1, 0), 1)/dx;  //Gradient needs to be divided by the grid-spacing
  GYB = space::BFD(glm::vec2(0, 1), 1)/dy;  //-``-

  solver.setTolerance(0.0000001);

  //Surface Integrators (i.e. convective fluxes over surface)
  XFLUX = space::FV_FLUX(glm::vec2(1, 0))/dx;
  YFLUX = space::FV_FLUX(glm::vec2(0, 1))/dy;

  //Diffusion Operator! (Note: This is in FV Form and Includes the Area / Volume!!!)
  DIFFUSIONMAT = space::FV_DIFFUSION(glm::vec2(1, 0))/dx/dx + space::FV_DIFFUSION(glm::vec2(0, 1))/dy/dy;
}

//Navier Stokes Matrices
smat VMAT;
smat E_MAT;
smat I_MAT;
VectorXd PSRCX;
VectorXd PSRCY;
VectorXd A;

VectorXd vXG;
VectorXd vYG;

VectorXd dP;
VectorXd dvX;
VectorXd dvY;

//SIMPLEC FV Algorithm
void navierstokes(float dt, float viscosity, VectorXd& vX, VectorXd& vY, VectorXd& P, VectorXd& bc, glm::vec2 g, glm::vec2 vt, Eigen::VectorXd& B){

  auto step = [&](){

    //Non-Linear Advection Operator
    VMAT = -1.0*(vX.asDiagonal()*XFLUX);
    VMAT += -1.0*(vY.asDiagonal()*YFLUX);

    //Explicit Integration Matrix
    E_MAT = bc.asDiagonal()*(VMAT + viscosity*DIFFUSIONMAT);

    //Pressure Source Terms
    PSRCX = bc.cwiseProduct(g.x*B.cwiseProduct((E*vt.x-vX).cwiseMax(0)));
    PSRCX.noalias() -= bc.cwiseProduct(XFLUX*P);

    PSRCY = bc.cwiseProduct(g.y*B.cwiseProduct((E*vt.y-vY).cwiseMax(0)));
    PSRCY.noalias() -= bc.cwiseProduct(YFLUX*P);

    //Implicit Portion
    I_MAT = SE - dt*E_MAT;
    A = (I_MAT*E).cwiseInverse();

  };

  //Execute Step
  step();

  //Crank Nicholson
  PDE::integrate<PDE::CN>(dt, vX, E_MAT, PSRCX);
  PDE::integrate<PDE::CN>(dt, vY, E_MAT, PSRCY);

  //Pressure Correction
  double newerr = 1.0;
  double pCorr = 1.0;
  int maxiter = 250;
  int n = 0;
  double err = 1.0;
  bool divergence = false;

  while(newerr > 1E-4 && pCorr > 1E-6 && !divergence && maxiter){

    n++;

    step();

    PSRCX = vX + dt*PSRCX;
    PSRCY = vY + dt*PSRCY;

    //Predict Velocity
    solver.compute(I_MAT);
    vXG = solver.solve(PSRCX);
    vYG = solver.solve(PSRCY);

    //2D Laplace Operator
    solver.compute((GXF*A.asDiagonal()*GXB+ GYF*A.asDiagonal()*GYB));

    //Compute Error Terms
    dP = solver.solve((GXB*vXG + GYB*vYG));
    dvX = -1.0*A.cwiseProduct(GXF*dP);
    dvY = -1.0*A.cwiseProduct(GYF*dP);

    //Pressure and Velocity Correction
    P.noalias() += bc.cwiseProduct(dP);
    vX = vXG + 0.8*bc.cwiseProduct(dvX); //Underrelax
    vY = vYG + 0.8*bc.cwiseProduct(dvY);

    //Divergence of Field as Error Quantification
    newerr = (GXB*vXG + GYB*vYG + dvY + dvX).squaredNorm()/(SIZE*SIZE);
    pCorr = dP.squaredNorm()/(SIZE*SIZE);

    if(newerr > err) divergence = true;
    maxiter--;

  }

}

void transport(double dt, double D, VectorXd& val, smat& VMAT, VectorXd& hc, VectorXd& src){
  TRANSPORTMAT = hc.asDiagonal()*VMAT;
  TRANSPORTMAT += D*hc.asDiagonal()*DIFFUSIONMAT;
  PDE::integrate<PDE::CN>(dt, val, TRANSPORTMAT, src);
}

};
