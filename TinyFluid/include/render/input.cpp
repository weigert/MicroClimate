#include "input.h"

void Input::handle(){
  //Do something with the input here!
  if( event.type == SDL_QUIT ) { quit = true; }
  else if( event.type == SDL_KEYDOWN ) {
    if(event.key.keysym.sym == SDLK_p){
      paused = !paused;
    }
    else if(event.key.keysym.sym == SDLK_t){
      trigger = true;
    }
    else if(event.key.keysym.sym == SDLK_SPACE){
      screen = (screen+1)%6;
    }
  }
}
