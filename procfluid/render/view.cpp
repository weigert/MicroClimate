#include "view.h"

/*
================================================================================
																SETUP / CLEANUP
================================================================================
*/

bool View::setup(){
  //Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) return false;

  //Create window
  gWindow = SDL_CreateWindow( WINDOW_NAME.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
  if( gWindow == NULL ) return false;

  //Prepare the Renderer
  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_GL_SetSwapInterval(false);

  return true;
}

void View::cleanup(){
  //Destroy window
	SDL_DestroyWindow( gWindow );
	SDL_DestroyRenderer (gRenderer);

	//Quit SDL subsystems
	SDL_Quit();
}

/*
================================================================================
																RENDER MASTER
================================================================================
*/

template<typename F, typename... Args>
void View::render(F function, Args&&... args){
	//Clear the Window
//	std::cout<<"Render ";
//	timer::benchmark<std::chrono::microseconds>([&](){

	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
  SDL_RenderClear(gRenderer);

	//Call whatever the user has specified...
	function(args...);

	//Present the Information
	SDL_RenderPresent(gRenderer);

//	});
}

/*
================================================================================
																DRAWING HELPERS
================================================================================
*/

void View::drawPixel(glm::ivec2 pos, glm::vec3 color, double opacity){
	/* Construct a Rect and Fill with Color at Position */
	int ratiox = SCREEN_WIDTH / SIZE;
	int ratioy = SCREEN_HEIGHT / SIZE;
  SDL_Rect rect{ratiox*pos.x, ratioy*pos.y, ratiox, ratioy};
  SDL_SetRenderDrawColor(gRenderer, 255*color.x, 255*color.y, 255*color.z, 255*opacity);
  SDL_RenderFillRect(gRenderer, &rect);
}

void View::drawPixel(glm::ivec2 pos, glm::vec3 color, double opacity, int _SIZE){
	/* Construct a Rect and Fill with Color at Position */
	int ratiox = SCREEN_WIDTH / _SIZE;
	int ratioy = SCREEN_HEIGHT / _SIZE;
  SDL_Rect rect{ratiox*pos.x, ratioy*pos.y, ratiox, ratioy};
  SDL_SetRenderDrawColor(gRenderer, 255*color.x, 255*color.y, 255*color.z, 255*opacity);
  SDL_RenderFillRect(gRenderer, &rect);
}


void View::drawLine(glm::vec2 pos, glm::vec2 dir){
	double scale = 2.0;
	int ratiox = SCREEN_WIDTH / SIZE;
	int ratioy = SCREEN_HEIGHT / SIZE;

	/* I need Direction AND Intensity */
	SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
	SDL_RenderDrawLine(gRenderer, ratiox*(pos.x+0.5-scale*0.5*dir.x), ratioy*(pos.y+0.5-scale*0.5*dir.y), ratiox*(pos.x+0.5+scale*0.5*dir.x), ratioy*(pos.y+0.5+scale*0.5*dir.y));
}
