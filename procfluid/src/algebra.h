/*
Author: Nicholas McDonald

Additional functions that allow for useful extension of Eigen for this purpose.
*/

typedef Eigen::Triplet<double> triplet;


//Modulus Operator
glm::vec2 operator%(const glm::vec2& rhs, const int x){
  return glm::mod(rhs, glm::vec2(x));
}

glm::ivec2 operator%(const glm::ivec2& rhs, const int x){
  return glm::mod(glm::vec2(rhs.x, rhs.y), glm::vec2(x));
}

std::ostream& operator<<(std::ostream& out, const glm::vec2 vec){
  out<<"X: "<<vec.x<<" Y: "<<vec.y;
  return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<double> vec){
  for(unsigned int i = 0; i < vec.size(); i++)
    out<<vec[i]<<" ";
  return out;
}


namespace alg{
  //Convert Index to Coordinates and Back
  glm::ivec2 pos(int i){
    return glm::ivec2(i/SIZE, i%SIZE);
  }

  int index(glm::vec2 pos){
    return pos.x*SIZE + pos.y;
  }

  int index(int x, int y){
    return x*SIZE + y;
  }

  Eigen::SparseMatrix<double> sparseIdentity(){
    Eigen::SparseMatrix<double> I(SIZE*SIZE, SIZE*SIZE);
    std::vector<triplet> list;
    for(int i = 0; i < SIZE*SIZE; i++){
      list.push_back(triplet(i, i, 1.0));
    }
    I.setFromTriplets(list.begin(), list.end());
    return I;
  }

  Eigen::SparseMatrix<double> sparseDiagonalize(Eigen::VectorXd &vec){
    Eigen::SparseMatrix<double> I(SIZE*SIZE, SIZE*SIZE);
    std::vector<triplet> list;
    for(int i = 0; i < SIZE*SIZE; i++){
      list.push_back(triplet(i, i, vec(i)));
    }
    I.setFromTriplets(list.begin(), list.end());
    return I;
  }

  void sparseDiagonalize(Eigen::VectorXd& vec, Eigen::SparseMatrix<double>& vmat){
    std::vector<triplet> list;
    for(int i = 0; i < SIZE*SIZE; i++){
      list.push_back(triplet(i, i, vec(i)));
    }
    vmat.setFromTriplets(list.begin(), list.end());
  }

}
