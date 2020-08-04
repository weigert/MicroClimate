/*
Author: Nicholas McDonald

Bezier-Curve based Coloring Schemes and Interpolator!

Stolen from my Territory Project

*/

namespace color{

  typedef std::vector<glm::vec3> scheme;

  scheme bw{
    glm::vec3(0.0, 0.0, 0.0),
    glm::vec3(1.0, 1.0, 1.0)
  };

  glm::vec3 red = glm::vec3(1.0, 0.0, 0.0);
  glm::vec3 white = glm::vec3(1.0);
  glm::vec3 black = glm::vec3(0.0);

  scheme green{
    glm::vec3(0.0, 1.0, 0.0)
  };

  scheme ocean{
    glm::vec3(0.18, 0.28, 0.35),
    glm::vec3(0.15, 0.41, 0.49),
    glm::vec3(0.00, 0.54, 0.60),
    glm::vec3(0.00, 0.68, 0.68),
    glm::vec3(0.07, 0.82, 0.71),
    glm::vec3(0.37, 0.96, 0.75)
  };

  scheme land{
    glm::vec3(0.18, 0.52, 0.34),
    glm::vec3(0.53, 0.69, 0.29),
    glm::vec3(0.75, 0.62, 0.23)
  };


  scheme nebula{
    glm::vec3(0.26, 0.33, 0.51),
    glm::vec3(0.50, 0.34, 0.59),
    glm::vec3(0.77, 0.30, 0.55),
    glm::vec3(0.96, 0.31, 0.38),
    glm::vec3(1.00, 0.46, 0.09)
  };

  scheme ivory{
    glm::vec3(0.18, 0.28, 0.35),
    glm::vec3(0.35, 0.40, 0.47),
    glm::vec3(0.52, 0.51, 0.59),
    glm::vec3(0.69, 0.64, 0.70),
    glm::vec3(0.85, 0.78, 0.82),
    glm::vec3(1.00, 0.93, 0.94)
  };

  //Multiply Color by Constant Factor
  glm::vec3 operator*(const double &d, const glm::vec3& vec) {
    return glm::vec3(d)*vec;
  }

  //De-Casteljau Algorithm
  glm::vec3 bezier(double t, scheme col){  //t between 0 and 1
    //Recursively do Linear Interpolations
    while(col.size() > 1){
      scheme next;
      for(unsigned int i = 0; i < col.size()-1; i++)
        next.push_back( (1.0-t)*col[i] + t*col[i+1] );
      col = next;
    }
    return col[0];
  }
}
