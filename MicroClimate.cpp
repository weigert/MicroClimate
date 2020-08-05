#include "TinyEngine/TinyEngine.h"

#define SIZE 100
#include "TinyFluid/TinyFluid.h"

#include "include/source.h"
#include "include/terrain.h"
#include "include/model.h"

int main( int argc, char* args[] ) {

	//Initialize a Window
	Tiny::view.vsync = false;
	Tiny::window("TinyFluid Climate Test", 1200, 800);

	int n = 8;
	omp_set_num_threads(n);
	Eigen::setNbThreads(n);

	bool paused = true;

	//Add the Event Handler
	Tiny::event.handler = [&](){
		if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_p)
			paused = !paused;
		if(Tiny::event.scroll.negy){
			zoom *= 0.99;
			projection = glm::ortho(-(float)Tiny::view.WIDTH*zoom, (float)Tiny::view.WIDTH*zoom, -(float)Tiny::view.HEIGHT*zoom, (float)Tiny::view.HEIGHT*zoom, -800.0f, 500.0f);
		}
		if(Tiny::event.scroll.posy){
			zoom /= 0.99;
			projection = glm::ortho(-(float)Tiny::view.WIDTH*zoom, (float)Tiny::view.WIDTH*zoom, -(float)Tiny::view.HEIGHT*zoom, (float)Tiny::view.HEIGHT*zoom, -800.0f, 500.0f);
		}
		if(Tiny::event.scroll.negx)
			camera = glm::rotate(camera, glm::radians(0.6f), glm::vec3(0.0f, 1.0f, 0.0f));
		if(Tiny::event.scroll.posx)
			camera = glm::rotate(camera, glm::radians(-0.5f), glm::vec3(0.0f, 1.0f, 0.0f));

	};

	Field field;
	field.SEED = 100;
	field.initialize();

	/*
			We want to pass the data to the shaders!

			Then we will give the model an additional property which we will later use.
	*/

	//Setup Shaders
	Shader shader({"shader/default.vs", "shader/default.gs", "shader/default.fs"}, {"in_Position", "in_Normal", "in_Color", "in_Cloud"});
	Shader depth({"shader/depth.vs", "shader/depth.fs"}, {"in_Position"});

	//Setup Rendering Billboards
  Billboard shadow(2000, 2000, false, true);
  Billboard image(1200, 800);
	Square2D flat;

	Model model(constructor, field.height.data());
	model.shift(glm::vec3(-SIZE/2, -150, -SIZE/2));				//Translate Mesh

	//Cloud Vector!
	Eigen::ArrayXf cloud;

	//std::cout<<"Threads Allocated: "<<Eigen::nbThreads()<<std::endl;

	//Add Climate System Attributes
	model.addBuffers(1);
	//model.bind(3, cloud.size(), 1, cloud.data());

	//Set up an ImGUI Interface here
	Tiny::view.interface = [&](){
	};

	//Define the rendering pipeline
	Tiny::view.pipeline = [&](){

		//Render Shadowmap
    shadow.target();                  //Prepare Target
    depth.use();                      //Prepare Shader
    model.model = glm::translate(glm::mat4(1.0), -viewPos);
    depth.uniform("dmvp", depthProjection * depthCamera * model.model);
    model.render(GL_TRIANGLES);       //Render Model

    //Regular Image
		Tiny::view.target(glm::vec3(0.8,1.0,1.0));    //Prepare Target

	  shader.use();                   //Prepare Shader
		shader.uniform("cloudpass", false);
    shader.texture("shadowMap", shadow.depth);
    shader.uniform("lightPos", lightPos);
    shader.uniform("lookDir", lookPos-cameraPos);
		shader.uniform("lightStrength", 0.2);
		shader.uniform("sealevel", field.sealevel);

    shader.uniform("vp", projection * camera);
    shader.uniform("dbmvp", biasMatrix * depthProjection * depthCamera * glm::mat4(1.0f));
    shader.uniform("model", model.model);
    model.render(GL_TRIANGLES);    //Render Model

		//Change a single boolean
		shader.uniform("cloudpass", true);
		model.render(GL_TRIANGLES);    //Render Model

	};

	std::vector<GLfloat> true_cloud;

	//Execute the render loop
	int t = 0;

	Tiny::loop([&](){

		camera = glm::rotate(camera, glm::radians(0.1f), glm::vec3(0.0f, 1.0f, 0.0f));

		//Compute Relevant Rendering Quantities
		//clouds2 = source::CLOUD(field.humidity, field.P, field.temperature, 150.0);


		if(!paused){

			camera = glm::rotate(camera, glm::radians(0.2f), glm::vec3(0.0f, 1.0f, 0.0f));

			timer::benchmark<std::chrono::microseconds>([&](){

				field.timestep();

			});

				cloud = (source::CLOUD(field.humidity, field.P, field.temperature, 100.0)).cast<float>();
				true_cloud.clear();

				for(int i = 0; i < dim.x-1; i++){
					for(int j = 0; j < dim.y-1; j++){
						for(int k = 0; k < 6; k++){
							true_cloud.push_back(cloud(i*dim.y+j)); //Jesus Christus
						}
					}
				}

				//This is a thing...

				glBindVertexArray(model.vao);
				glBindBuffer(GL_ARRAY_BUFFER, model.vbo[3]);
				glBufferData(GL_ARRAY_BUFFER, true_cloud.size()*sizeof(GLfloat), &true_cloud[0], GL_STATIC_DRAW);
				//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, cloud.size()*sizeof(float), cloud.data());
				glEnableVertexAttribArray(3);
				glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);

		}

	});

	Tiny::quit();

	return 0;
}
