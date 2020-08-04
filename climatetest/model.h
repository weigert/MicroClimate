//Model Stuff
int SEED = 10;
double scale = 15.0;
glm::vec2 dim = glm::vec2(100);
noise::module::Perlin perlin;

//View Stuff
const int WIDTH = 1200;
const int HEIGHT = 800;

glm::vec3 viewPos = glm::vec3(50, 0, 50);
float zoom = 0.1;
float zoomInc = 0.001;
float rotation = 0.0f;
glm::vec3 cameraPos = glm::vec3(50, 60, 50);
glm::vec3 lookPos = glm::vec3(0, 10, 0);
glm::mat4 camera = glm::lookAt(cameraPos, lookPos, glm::vec3(0,1,0));
glm::mat4 projection = glm::ortho(-(float)WIDTH*zoom, (float)WIDTH*zoom, -(float)HEIGHT*zoom, (float)HEIGHT*zoom, -800.0f, 500.0f);

glm::vec3 lightPos = glm::vec3(-100.0f, 100.0f, -150.0f);

glm::mat4 depthModelMatrix = glm::mat4(1.0);
glm::mat4 depthProjection = glm::ortho<float>(-300, 300, -300, 300, 0, 800);
glm::mat4 depthCamera = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0,1,0));

glm::mat4 biasMatrix = glm::mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

std::function<void(Model*, double* )> constructor = [&](Model* m, double* D){

  //Clear the Containers
  m->indices.clear();
  m->positions.clear();
  m->normals.clear();
  m->colors.clear();

  int w = dim.y;

  //Loop over all positions and add the triangles!
  for(int i = 0; i < dim.x-1; i++){
    for(int j = 0; j < dim.y-1; j++){

      //Add to Position Vector
      glm::vec3 b = glm::vec3(i, scale*D[i*w+j], j);
      glm::vec3 a = glm::vec3(i+1, scale*D[(i+1)*w+j], j);
      glm::vec3 d = glm::vec3(i, scale*D[i*w+j+1], j+1);
      glm::vec3 c = glm::vec3(i+1, scale*D[(i+1)*w+j+1], j+1);

      //Add Indices
      m->indices.push_back(m->positions.size()/3+0);
      m->indices.push_back(m->positions.size()/3+1);
      m->indices.push_back(m->positions.size()/3+2);

      m->positions.push_back(a.x);
      m->positions.push_back(a.y);
      m->positions.push_back(a.z);
      m->positions.push_back(b.x);
      m->positions.push_back(b.y);
      m->positions.push_back(b.z);
      m->positions.push_back(c.x);
      m->positions.push_back(c.y);
      m->positions.push_back(c.z);

      glm::vec3 n1 = -glm::normalize(glm::cross(a-b, c-b));

      for(int i = 0; i < 3; i++){
        m->normals.push_back(n1.x);
        m->normals.push_back(n1.y);
        m->normals.push_back(n1.z);
      }

      m->indices.push_back(m->positions.size()/3+0);
      m->indices.push_back(m->positions.size()/3+1);
      m->indices.push_back(m->positions.size()/3+2);

      m->positions.push_back(d.x);
      m->positions.push_back(d.y);
      m->positions.push_back(d.z);
      m->positions.push_back(c.x);
      m->positions.push_back(c.y);
      m->positions.push_back(c.z);
      m->positions.push_back(b.x);
      m->positions.push_back(b.y);
      m->positions.push_back(b.z);

      glm::vec3 n2 = -glm::normalize(glm::cross(d-c, b-c));

      for(int i = 0; i < 3; i++){
        m->normals.push_back(n2.x);
        m->normals.push_back(n2.y);
        m->normals.push_back(n2.z);
      }
    }
  }
};
