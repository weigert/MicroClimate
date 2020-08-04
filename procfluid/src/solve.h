/*
  Full Navier-Stokes / Transport Equation Solvers!
*/

namespace PDE{

  enum Solver{
    TRANSPORT,
    SIMPLEC
  };


  //Full-NS-Solver
  template<Solver S>
  void solveNavierStokes(Eigen::VectorXd& vX, Eigen::VectorXd& vY, Eigen::VectorXd& P, Eigen::SparseMatrix<double> B, double dt, glm::vec2 g, double tol, int maxiter, double relaxP, double relaxV){
    std::cout<<"Please specifiy a valid solution method."<<std::endl;
  }

  //Set a few variables...
  Eigen::SparseMatrix<double> GXF;       //Gradient Operator X-Element
  Eigen::SparseMatrix<double> GYF;       //Gradient Operator Y-Element
  Eigen::SparseMatrix<double> GXB;       //Gradient Operator X-Element
  Eigen::SparseMatrix<double> GYB;       //Gradient Operator Y-Element
  Eigen::SparseMatrix<double> XFLUX;         //Integrated Surface Flux X, UNIFORM AREA!!
  Eigen::SparseMatrix<double> YFLUX;
  Eigen::SparseMatrix<double> XDIFFUSION;    //Integrated Surface Flux X, UNIFORM AREA!!
  Eigen::SparseMatrix<double> YDIFFUSION;
  Eigen::VectorXd E;

  void initialize(double dx, double dy){
    E = Eigen::ArrayXd::Ones(SIZE*SIZE);

    //Discrete Differential Operators (of some order)
    GXF = space::FFD(glm::vec2(1, 0), 1)/dx;  //Gradient needs to be divided by the grid-spacing
    GYF = space::FFD(glm::vec2(0, 1), 1)/dy;  //-``-
    GXB = space::BFD(glm::vec2(1, 0), 1)/dx;  //Gradient needs to be divided by the grid-spacing
    GYB = space::BFD(glm::vec2(0, 1), 1)/dy;  //-``-

    //Discrete Differential Operator (Laplace)
    solver.compute(GXF*GXB + GYF*GYB);
    solver.setTolerance(0.00001);

    //Surface Integrators (i.e. convective fluxes over surface)
    XFLUX = space::FV_FLUX(glm::vec2(1, 0))/dx;
    YFLUX = space::FV_FLUX(glm::vec2(0, 1))/dy;

    //Diffusion Operator! (Note: This is in FV Form and Includes the Area / Volume!!!)
    XDIFFUSION = space::FV_DIFFUSION(glm::vec2(1, 0))/dx/dx;
    YDIFFUSION = space::FV_DIFFUSION(glm::vec2(0, 1))/dy/dy;
  }

  template<> //Naive Solution (Strange Pressure Behavior)
  void solveNavierStokes<SIMPLEC>(Eigen::VectorXd& vX, Eigen::VectorXd& vY, Eigen::VectorXd& P, Eigen::SparseMatrix<double> B, double dt, glm::vec2 g, double tol, int maxiter, double relaxP, double relaxV){
    double viscosity = 0.00014;

    Eigen::SparseMatrix<double> vX_MAT = alg::sparseDiagonalize(vX);
    Eigen::SparseMatrix<double> vY_MAT = alg::sparseDiagonalize(vY);
    Eigen::SparseMatrix<double> E_MAT = -vX_MAT*XFLUX - vY_MAT*YFLUX + viscosity*(XDIFFUSION + YDIFFUSION);

    Eigen::VectorXd P_SOURCE_X;
    Eigen::VectorXd P_SOURCE_Y;
    P_SOURCE_X = g.x*Eigen::ArrayXd::Ones(SIZE*SIZE);
    P_SOURCE_Y = g.y*Eigen::ArrayXd::Ones(SIZE*SIZE);
    P_SOURCE_X += -XFLUX*P;
    P_SOURCE_Y += -YFLUX*P;

    //Implicit Solution
    Eigen::SparseMatrix<double> I_MAT_X = (alg::sparseIdentity() - dt*B*E_MAT);
    Eigen::SparseMatrix<double> I_MAT_Y = (alg::sparseIdentity() - dt*B*E_MAT);

    //Helper Values (Sum over Rows)
    Eigen::VectorXd A_X = I_MAT_X*E;
    Eigen::VectorXd A_Y = I_MAT_Y*E;

    //All Elements!
    Eigen::VectorXd UA_X = E.cwiseQuotient(A_X);
    Eigen::VectorXd UA_Y = E.cwiseQuotient(A_Y);

    Eigen::VectorXd dP;
    Eigen::VectorXd dvX;
    Eigen::VectorXd dvY;

    Eigen::VectorXd vXG;
    Eigen::VectorXd vYG;

    //Implicit Timestep
    solver.compute(I_MAT_X);
    vX = solver.solve(vX+dt*B*P_SOURCE_X);
    solver.compute(I_MAT_Y);
    vY = solver.solve(vY+dt*B*P_SOURCE_Y);

    //Convergence
    double newerr = 1.0;
    bool divergence = false;

    while(newerr > tol && !divergence && maxiter){

      vX_MAT = alg::sparseDiagonalize(vX);
      vY_MAT = alg::sparseDiagonalize(vY);
      E_MAT = -vX_MAT*XFLUX - vY_MAT*YFLUX + viscosity*(XDIFFUSION + YDIFFUSION);

      P_SOURCE_X = g.x*Eigen::ArrayXd::Ones(SIZE*SIZE);
      P_SOURCE_Y = g.y*Eigen::ArrayXd::Ones(SIZE*SIZE);
      P_SOURCE_X += -XFLUX*P;
      P_SOURCE_Y += -YFLUX*P;

      I_MAT_X = (alg::sparseIdentity() - dt*B*E_MAT);
      I_MAT_Y = (alg::sparseIdentity() - dt*B*E_MAT);
      A_X = I_MAT_X*E;
      A_Y = I_MAT_Y*E;
      UA_X = E.cwiseQuotient(A_X);
      UA_Y = E.cwiseQuotient(A_Y);

      solver.compute(I_MAT_X);
      vXG = solver.solve(vX+dt*B*P_SOURCE_X);
      solver.compute(I_MAT_Y);
      vYG = solver.solve(vY+dt*B*P_SOURCE_Y);

      //Pressure Correction
      solver.compute((GXF*alg::sparseDiagonalize(UA_X)*GXB+ GYF*alg::sparseDiagonalize(UA_Y)*GYB)); //2D Laplace Operator
      dP = solver.solve(GXB*vX + GYB*vY);

      // We compute the velocity correction based on the pressure correction

      dvX = -(GXF*dP).cwiseProduct(UA_X);
      dvY = -(GYF*dP).cwiseProduct(UA_Y);

      // We correct our velocity guesses from the intermediary field

      vX = vXG + relaxV*B*dvX;
      vY = vYG + relaxV*B*dvY;
      P += relaxP*B*dP;

      // Compute the Error (Divergence of Field) simply using our guess.
      newerr = (GXB*vX + GYB*vY).squaredNorm()/(SIZE*SIZE);
      maxiter--;

      //std::cout<<newerr<<std::endl;
      if(newerr > 1E+5) divergence = true;
    }

    //Error Handling
    if(!maxiter) std::cout<<"Max iterations surpassed."<<std::endl;
    if(divergence) std::cout<<"Instability encountered."<<std::endl;
  };

  /*
    Transport Equation Integrator:

      Define your own parameter set...

  */

  void solveTransport(double dt, double dx, double dy, Eigen::VectorXd& val, Eigen::VectorXd& vX, Eigen::VectorXd& vY){
    double diffusivity = 0.001;

    //Velocity Matrices
    Eigen::SparseMatrix<double> vX_MAT = alg::sparseDiagonalize(vX);
    Eigen::SparseMatrix<double> vY_MAT = alg::sparseDiagonalize(vY);

    Eigen::SparseMatrix<double> XFLUX = space::FV_FLUX(glm::vec2(1, 0))/dx;
    Eigen::SparseMatrix<double> YFLUX = space::FV_FLUX(glm::vec2(0, 1))/dy;

    Eigen::SparseMatrix<double> XDIFFUSION = space::FV_DIFFUSION(glm::vec2(1, 0))/dx/dx;
    Eigen::SparseMatrix<double> YDIFFUSION = space::FV_DIFFUSION(glm::vec2(0, 1))/dy/dy;

    //Get our Operators
    Eigen::SparseMatrix<double> CONVECTION = -vX_MAT*XFLUX - vY_MAT*YFLUX;
    Eigen::SparseMatrix<double> DIFFUSION = diffusivity*(XDIFFUSION + YDIFFUSION);

    //Full Matrix Operator...
    Eigen::SparseMatrix<double> MAT = CONVECTION + DIFFUSION;

    //Integrate!
    PDE::integrate<PDE::CN>(dt, val, MAT);
  }
};
