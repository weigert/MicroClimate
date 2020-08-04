/*

  Time Integrators

*/


#include<Eigen/SparseLU>

namespace PDE{

using namespace Eigen;

  enum Integrator{
    EE, //Explicit Euler
    IE, //Implicit Euler
    CN, //Crank-Nicholson
    PI  //Point-Implicit
  };

  BiCGSTAB<SparseMatrix<double>> solver;
  SparseMatrix<double> SE = alg::sparseIdentity();

  /* Pure Implicit / Explicit / Defined Mixture */
  template<Integrator I>
  void integrate(double dt, VectorXd& val, SparseMatrix<double>& mat){
    std::cout<<"Please specify an integration method."<<std::endl;
  }

  /* Pure Implicit / Explicit / Defined Mixture + Source Term */
  template<Integrator I>
  void integrate(double dt, VectorXd& val, SparseMatrix<double>& mat, VectorXd& source){
    std::cout<<"Please specify an integration method."<<std::endl;
  }

  /* Point Implicit / Arbitrary Mixture */
  template<Integrator I>
  void integrate(double dt, VectorXd& val, SparseMatrix<double>& mat_E, SparseMatrix<double>& mat_I){
    std::cout<<"Please specify an integration method."<<std::endl;
  }

  template<Integrator I>
  void integrate(double dt, VectorXd& val, SparseMatrix<double>& mat_E, SparseMatrix<double>& mat_I, VectorXd& source){
    std::cout<<"Please specify an integration method."<<std::endl;
  }

  /* Explicit Euler Integrator - Fully Explicit */

  template<>
  void integrate<EE>(double dt, VectorXd& val, SparseMatrix<double>& mat){
    val += dt*mat*val;
  }

  template<>
  void integrate<EE>(double dt, VectorXd& val, SparseMatrix<double>& mat, VectorXd& source){
    val += dt*(mat*val + source);
  }

  /* Implicit Euler Integrator - Fully Implicit */

  template<>
  void integrate<IE>(double dt, VectorXd& val, SparseMatrix<double>& mat){
    solver.compute(SE-dt*mat);
    val.noalias() = solver.solve(val);
  }

  template<>
  void integrate<IE>(double dt, VectorXd& val, SparseMatrix<double>& mat, VectorXd& source){
    solver.compute(SE-dt*mat);
    val.noalias() = solver.solve(val + dt*source);
  }

  /* Crank-Nicholson Integrator - Semi Explicit / Implicit */

  template<>
  void integrate<CN>(double dt, VectorXd& val, SparseMatrix<double>& mat){
    integrate<EE>(0.5*dt, val, mat); //Perform Explicit Half-Step
    integrate<IE>(0.5*dt, val, mat); //Perform Implicit Half-Step
  }

  template<>
  void integrate<CN>(double dt, VectorXd& val, SparseMatrix<double>& mat, VectorXd& source){
    integrate<EE>(0.5*dt, val, mat, source); //Perform Explicit Half-Step
    integrate<IE>(0.5*dt, val, mat, source); //Perform Implicit Half-Step
  }

  /* Point-Implicit Integrator */

  template<>
  void integrate<PI>(double dt, VectorXd& val, SparseMatrix<double>& mat_E, SparseMatrix<double>& mat_I){
    integrate<EE>(dt, val, mat_E); //Perform Explicit Integration
    integrate<IE>(dt, val, mat_I); //Perform Implicit Half-Step
  }

  template<>
  void integrate<PI>(double dt, VectorXd& val, SparseMatrix<double>& mat_E, SparseMatrix<double>& mat_I, VectorXd& source){
    integrate<EE>(dt, val, mat_E, source); //Perform Explicit Integration
    integrate<IE>(dt, val, mat_I, source); //Perform Implicit Half-Step
  }

  /* Feeding 0.5 Mat for Both is equivalent to CN! */
  /* You don't even need to feed 0.5 in theory! */
};
