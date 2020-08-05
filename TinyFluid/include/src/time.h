//Time Integrators
namespace PDE{
using namespace Eigen;
typedef SparseMatrix<double,RowMajor> smat;

enum Integrator{
  EE, //Explicit Euler
  IE, //Implicit Euler
  CN, //Crank-Nicholson
  PI  //Point-Implicit
};

//To-Do: General Multi-Step Integrators

BiCGSTAB<SparseMatrix<double,RowMajor>> solver;
SparseMatrix<double,RowMajor> SE = alg::sparseIdentity();

/* Pure Implicit / Explicit / Defined Mixture */
template<Integrator I>
void integrate(double dt, VectorXd& val, smat& mat){
  std::cout<<"Please specify an integration method."<<std::endl;
}

/* Pure Implicit / Explicit / Defined Mixture + Source Term */
template<Integrator I>
void integrate(double dt, VectorXd& val, smat& mat, VectorXd& src){
  std::cout<<"Please specify an integration method."<<std::endl;
}

/* Point Implicit / Arbitrary Mixture */
template<Integrator I>
void integrate(double dt, VectorXd& val, smat& mat_E, smat& mat_I){
  std::cout<<"Please specify an integration method."<<std::endl;
}

/* Point Implicit / Arbitrary Mixture + Source Term */
template<Integrator I>
void integrate(double dt, VectorXd& val, smat& mat_E, smat& mat_I, VectorXd& src){
  std::cout<<"Please specify an integration method."<<std::endl;
}

/* Explicit Euler Integrator - Fully Explicit */

template<>
void integrate<EE>(double dt, VectorXd& val, smat& mat){
  val.noalias() += dt*(mat*val);
}

template<>
void integrate<EE>(double dt, VectorXd& val, smat& mat, VectorXd& src){
  val.noalias() += dt*(mat*val);
  val.noalias() += dt*src;
}

/* Implicit Euler Integrator - Fully Implicit */

template<>
void integrate<IE>(double dt, VectorXd& val, smat& mat){
  solver.compute(SE-dt*mat);
  val= solver.solve(val);
}

template<>
void integrate<IE>(double dt, VectorXd& val, smat& mat, VectorXd& source){
  solver.compute(SE-dt*mat);
  val = solver.solve(val + dt*source);
}

/* Crank-Nicholson Integrator - Semi Explicit / Implicit */

template<>
void integrate<CN>(double dt, VectorXd& val, smat& mat){
  integrate<EE>(0.5*dt, val, mat); //Perform Explicit Half-Step
  integrate<IE>(0.5*dt, val, mat); //Perform Implicit Half-Step
}

template<>
void integrate<CN>(double dt, VectorXd& val, smat& mat, VectorXd& source){
  integrate<EE>(0.5*dt, val, mat, source); //Perform Explicit Half-Step
  integrate<IE>(0.5*dt, val, mat, source); //Perform Implicit Half-Step
}

/* Point-Implicit Integrator */

template<>
void integrate<PI>(double dt, VectorXd& val, smat& mat_E, smat& mat_I){
  integrate<EE>(dt, val, mat_E); //Perform Explicit Integration
  integrate<IE>(dt, val, mat_I); //Perform Implicit Half-Step
}

template<>
void integrate<PI>(double dt, VectorXd& val, smat& mat_E, smat& mat_I, VectorXd& source){
  integrate<EE>(dt, val, mat_E, source); //Perform Explicit Integration
  integrate<IE>(dt, val, mat_I, source); //Perform Implicit Half-Step
}

//End of Namespace
};
