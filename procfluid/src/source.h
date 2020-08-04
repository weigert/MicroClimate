/*
  Physical Source Terms

  Desirable Terms:
  - Henry's Law for Vapor equilibrium concentration
  - Raoults Law for Determining Phase Fraction!
  - Thin Film Mass Transfer Law
  - Sun Heating (Flat or Radial)
  -
*/

namespace source{

  Eigen::ArrayXd E = Eigen::ArrayXd::Ones(SIZE*SIZE);

  Eigen::ArrayXd Psat(Eigen::VectorXd& P, Eigen::VectorXd& T){
    Eigen::ArrayXd _psat = Eigen::ArrayXd::Ones(SIZE*SIZE);
    for(int i = 0; i < SIZE*SIZE; i++)
      _psat(i) = pow(10.0, 8.07131 - 1730.63/(233.426+T(i)));
    return _psat*101325.0/760.0;
  }

  Eigen::ArrayXd Ps;

  //How do clouds look?
  Eigen::ArrayXd CLOUD(Eigen::VectorXd& H, Eigen::VectorXd& P, Eigen::VectorXd& T, double thresh){
    return 0.9*(thresh*E-(Psat(P, T)-H.array()*P.array()).cwiseMin(thresh))/thresh; //It is possible to be "oversaturated"
  }

  Eigen::ArrayXd G = shape::flatGaussian(glm::vec2(SIZE/2.0, SIZE/2.0), glm::vec2(SIZE*15.0, SIZE*2.0));

  Eigen::VectorXd TSOURCE(Eigen::VectorXd &B, Eigen::VectorXd& height, double s, Eigen::VectorXd& H, Eigen::VectorXd& P, Eigen::VectorXd& T, Eigen::VectorXd& vX, Eigen::VectorXd& vY){

    Eigen::ArrayXd DP = Ps-H.array()*P.array();

    //Evaporation Cooling (MORE!)
    const double m = 0.001;
    Eigen::ArrayXd Z = -50000.0*m*DP.cwiseMax(0.0)*(s*E-height.array().cwiseMin(s))/Ps;

    //Rain Removal
    const double thresh = 75.0; //This is the oversaturation capacity
    const double k = 0.0001;
    Z += 1000.0*k*(DP.cwiseMin(thresh)-E*thresh);

    //Boundary Volume Not Height Difference... (Increase proportionally to the temperature...)
    Z -= 50.0*0.5*(vX.cwiseProduct((PDE::GXF+PDE::GXB)*height.cwiseMax(s)) + vY.cwiseProduct((PDE::GYF+PDE::GYB)*height.cwiseMax(s))).array();

    //Radiation Transfer (Get Albedo and Radiation Balance Steady State T)
    Z += 0.1*(10*E + 15*G - T.array())*(E-CLOUD(H, P, T, 250.0));

    return Z;
  }

  Eigen::ArrayXd HSOURCE(Eigen::VectorXd& B, Eigen::VectorXd& height, double s, Eigen::VectorXd& H, Eigen::VectorXd& P, Eigen::VectorXd& T){

    //Compute Psat one time! (call hsource before tsource)
    Ps = Psat(P, T);

    //Mass Transfer Addition..
    //Evaporation Mass Transfer (k = approach rate to th equilibrium concentration)
    const double m = 0.001;
    Eigen::ArrayXd Z = m*(Ps-H.array()*P.array()).cwiseMax(0.1)*B.array()/Ps;

    //Rain Removal
    const double thresh = 75.0; //This is the oversaturation capacity!
    const double k = 0.0001;
    Z += -k*E*thresh + k*(Ps-H.array()*P.array()).cwiseMin(thresh);

    return Z;
  }
};
