/*
Author: Nicholas McDonald

Spatial Discretization Approximations and Helpers!

*/

#include <math.h>

namespace space{

/*
================================================================================
                              MAIN LINALG HELPERS
================================================================================
*/

//Factorial Calculator
double fac(int k){
  int j = 1;  //Factorial
  for(int i = 1; i <= k; i++)
    j*= i;
  return j;
}

//Taylor Coefficient
double taylor(double x, double a, int n){
  return (pow(x-a, n) / fac(n));
}

/*
  There are three main operator types I use:

    - Interpolators   (i.e. lagrange polynomials)
    - Differentiators (i.e. finite differences)
    - Integrators     (i.e. numerical quadrature)

  As they are linearizations, they can be put into general matrix form after
  the individual operator weights have been computed.

  Below are weight calculating functions for all three types of operators, as
  well as one general function for placing them into a sparse matrix system.
*/

/*
  Lagrange Interpolator:
    Setup the Vandermonde Matrix for the linear equation of the polynomial.
    Inversion and evaluation at point = 0 yields the linear combination of
    evaluated values f, that gives us point a0 (the intercept), which is
    our interpolated value for arbitrary polynomial degree.
*/

std::vector<double> LI(std::initializer_list<double> shift){
  std::vector<double> weights = shift;

  //Vandermonde Matrix
  int N = shift.size();
  Eigen::MatrixXd V(N, N);
  for(int p = 0; p < N; p++)
    for(int d = 0; d < N; d++)
      V(p, d) = pow(weights[p], d);

  //Invert the Matrix
  V = V.inverse();

  //Extract the 0th Row
  for(int i = 0; i < N; i++)
    weights[i] = V(0, i);

  return weights;
}

/*
  Finite Difference Approximator:
    Compute the Taylor Matrix for all points expanded around zero
    Invert the matrix and select the appropriate row for the weights!
*/

std::vector<double> FD(std::initializer_list<double> points, unsigned int order){
  std::vector<double> weights = points;

  //Check for Consistency
  if(order >= points.size()){
    std::cout<<"Order must be strictly smaller than the number of support points."<<std::endl;
    return weights;
  }

  //Taylor Matrix
  int N = points.size();
  Eigen::MatrixXd T(N, N);
  for(int p = 0; p < N; p++)
    for(int d = 0; d < N; d++)
      T(p, d) = taylor(weights[p], 0, d);

  //Invert the Matrix
  T = T.inverse();

  //Extract the Order'th Row
  for(int i = 0; i < N; i++)
    weights[i] = T(order, i);

  return weights;
}

/*
  To Do: Numerical Quadrature Approximator
*/

/*
  General Sparse Operator Matrix:
    Quadrature, Finite Differences and Lagrange Interpolation can all be considered
    a linearization of these mathematical operators from known values.
    Therefore they can be written in matrix form. This is that.
    Specify the positions _p where you want weights w, which you can retreive
    from the weight generating functions above.
    You also must specifiy the dimension (i.e. vector (1, 0) or (0, 1))
    Currently this handles ONLY periodic boundary conditions.
*/

Eigen::SparseMatrix<double> OP(std::initializer_list<double> _p, std::vector<double> w, glm::ivec2 dim){
  Eigen::SparseMatrix<double> M(SIZE*SIZE, SIZE*SIZE);
  std::vector<triplet> list;
  std::vector<double> p = _p;

  //Loop over all Guys
  for(int i = 0; i < SIZE*SIZE; i++)
    //Loop over all Offset Positions
    for(unsigned int j = 0; j < p.size(); j++){
      //Position of the Offset Element and add the weight to the matrix
      glm::ivec2 shifted = (alg::pos(i) + glm::ivec2(p[j])*dim + SIZE)%SIZE;
      list.push_back(triplet(i, alg::index(shifted), w[j]));
    }
  M.setFromTriplets(list.begin(), list.end());
  return M;
}

/*
================================================================================
                      EXAMPLE OPERATORS FOR CONVENIENCE
================================================================================
*/

/*
  These individual operators can be tuned to any accuracy you like.
*/

//Interpolate the surface values linearly on both faces in a direction!
Eigen::SparseMatrix<double> FV_FLUX(glm::vec2 dim){
  std::vector<double> w = LI({-0.5, 0.5});
  return OP({0, 1}, w, dim) -  OP({0, -1}, w, dim);
}

//1st Order Accurate Forward Finite Differences, Differential at Boundary for
//both surfaces in a direction summed up!
Eigen::SparseMatrix<double> FV_DIFFUSION(glm::vec2 dim){
  std::vector<double> w1 = FD({0, 1}, 1);
  std::vector<double> w2 = FD({0, -1}, 1);
  return OP({0, 1}, w1, dim) -  OP({0, -1}, w2, dim);
}

//4th Order Accurate Centered Differences, Approximation of Nth order Derivative
Eigen::SparseMatrix<double> CFD(glm::vec2 dim, int n){
  std::vector<double> w = FD({-2, -1, 0, 1, 2}, n);
  return OP({-2, -1, 0, 1, 2}, w, dim);
}

//Forward Finite Differences
Eigen::SparseMatrix<double> FFD(glm::vec2 dim, int n){
  std::vector<double> w = FD({0, 1}, n);
  return OP({0, 1}, w, dim);
}

//Forward Finite Differences
Eigen::SparseMatrix<double> BFD(glm::vec2 dim, int n){
  std::vector<double> w = FD({0, 1}, n);
  return OP({-1, 0}, w, dim);
}

//Forward Finite Differences
Eigen::SparseMatrix<double> SFD(glm::vec2 dim, int n){
  std::vector<double> w = FD({-1, 0, 1}, n);
  return OP({-1, 0, 1}, w, dim);
}

}; //End of Namespace
