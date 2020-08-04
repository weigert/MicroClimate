class Input{
public:
  bool quit = false;
  bool paused = true;
  bool trigger = false;
  int screen = 0;

  void handle();
  SDL_Event event;
};
