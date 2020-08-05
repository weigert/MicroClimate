//Shape Generators
namespace shape{
using namespace Eigen;

ArrayXd flatGaussian(glm::vec2 mean, double stddev){
  ArrayXd gauss = ArrayXd::Zero(SIZE*SIZE);
  for(int i = 0; i < SIZE*SIZE; i++){
    glm::vec2 _pos = alg::pos(i);
    gauss(i) = exp(-(pow(_pos.x - mean.x, 2) + pow(_pos.y - mean.y, 2))/stddev);
  }
  return gauss;
}

VectorXd flatGaussian(glm::vec2 mean, glm::vec2 stddev){
  VectorXd gauss = ArrayXd::Zero(SIZE*SIZE);
  for(int i = 0; i < SIZE*SIZE; i++){
    glm::vec2 _pos = alg::pos(i);
    gauss(i) = exp(-(pow(_pos.x - mean.x, 2)/stddev.x + pow(_pos.y - mean.y, 2)/stddev.y));
  }
  return gauss;
}

VectorXd circle(glm::vec2 center, double rad){
  VectorXd circ = ArrayXd::Zero(SIZE*SIZE);
  for(int i = 0; i < SIZE*SIZE; i++){
    glm::vec2 _pos = alg::pos(i);
    if(sqrt(pow(_pos.x-center.x, 2) + pow(_pos.y-center.y, 2)) < rad)
      circ(i) = 1.0;
  }
  return circ;
}

VectorXd threshold(VectorXd &val, double thresh, bool greater){
  VectorXd threshed = ArrayXd::Ones(SIZE*SIZE);
  for(int i = 0; i < SIZE*SIZE; i++){
    if(greater) threshed(i) = (val(i) > thresh)?1.0:0.0;
    else threshed(i) = (val(i) > thresh)?0.0:1.0;
  }
  return threshed;
}

};
