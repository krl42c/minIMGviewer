#include "application.h"

void MinIMGView::init_sdl(Application &app) {
  //TODO: error checking
  SDL_Init(SDL_INIT_VIDEO);
  IMG_Init(IMG_INIT_PNG);
  app.window = SDL_CreateWindow("minimgview", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1600, 900, SDL_WINDOW_RESIZABLE);
  app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
}

void MinIMGView::clean_up(Application &app) {
  SDL_DestroyRenderer(app.renderer);
  SDL_DestroyWindow(app.window);
  IMG_Quit();
  SDL_Quit();
}

MinIMGView::Image MinIMGView::create_image(std::string path) {
  /* @Refactor Although i made this struct when i started to write the program, it may be cleaner without using it and directly rendering files
   * from a given path.
   */
  Image img; 
  img.path = path;
  img.x = 0;
  img.y = 0;
  img.dest.x = img.x;
  img.dest.y = img.y;
  return img;
}

void MinIMGView::render(Application &app, Image &img) {
  /* This is here to avoid memory leaking when skipping images */
  if(app.renderer != nullptr)
    SDL_DestroyRenderer(app.renderer);
  
  app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
  img.texture = load_texture(img.path, app);
 
  /* 
    This may look "hacky", but i think it's perfectly fine, however i'll think of a better solution.
    An image will never be modified on it's first load, so we are fine to assume SDL_QueryTexture will be called on each
    image load, and the origin SDL_Rect data will be stored in original_val (we store this so we can render to the same window size when modifying the image).
  */

  if(img.mod) {
    SDL_SetWindowSize(app.window, img.original_val.w, img.original_val.h);
  } else {
    SDL_QueryTexture(img.texture, nullptr, nullptr, &img.dest.w, &img.dest.h);
    SDL_SetWindowSize(app.window, img.dest.w, img.dest.h);
    img.original_val = img.dest;
  }

  SDL_RenderCopy(app.renderer, img.texture, nullptr, &img.dest);
  SDL_RenderPresent(app.renderer);
}


constexpr void MinIMGView::zoom_in(Image &img, int x, int y) {
  img.mod = true;
  img.dest.x = x - (img.dest.w * 2 - img.dest.w) / 2;
  img.dest.y = y - (img.dest.h * 2 - img.dest.h) / 2;        
  img.dest.w *= 1.2f;
  img.dest.h *= 1.2f;
}

constexpr void MinIMGView::zoom_out(Image &img, int x, int y) {
  img.mod = true;
  //TODO: this calculation doesn't work correctly
  img.dest.x = x - (img.dest.w * 2 - img.dest.w) / 2;
  img.dest.y = y - (img.dest.h * 2 - img.dest.h) / 2;        
  img.dest.w /= 1.2f;
  img.dest.h /= 1.2f;
}

void MinIMGView::run(Application &app, std::string path) {
  SDL_Event ev;
  int current = 0;
  int mouse_x, mouse_y;
  bool quit = false;

  std::string wd;
  std::vector<std::string> file_list;
  Image img;

  /* If we receive a file as an argument, create an image from it and get the rest of the files in that directory,
   * however if we receive a folder, get all the files from the folder and render the first one.
   */
  if(path.find(".") != std::string::npos) {
    img = create_image(path);
    wd = get_wd(path);
    file_list = load_from_wd(wd);

  } else {
    file_list = load_from_wd(path);
    if(!file_list.empty())
      img = create_image(file_list.at(0));
    current++;
  }

  if(file_list.empty()) {
    quit = true;
    printf("Provided directory doesn't contain any images, exitting.\n");
  }
  else
    render(app, img);

  while(!quit) {
  while(SDL_PollEvent(&ev)) {
    if(ev.type == SDL_QUIT)
      quit = true;
    if(ev.type == SDL_KEYDOWN) {
      switch(ev.key.keysym.sym) {
        case SDLK_RIGHT:
          if(current < (int)file_list.size()) {
            current++;
            if(current == (int)file_list.size() -1)
              current = 0;
            img = create_image(file_list.at(current));
          } else {
            current = 0;
            SDL_RenderClear(app.renderer);
          }
          break;
        case SDLK_LEFT:
          if(current == 0)
            current = (int)file_list.size() - 1;
          current--;
          img = create_image(file_list.at(current));
          break;
        case SDLK_z:
          SDL_GetMouseState(&mouse_x, &mouse_y);
          zoom_in(img, mouse_x, mouse_y);
          break;
        case SDLK_x:
          SDL_GetMouseState(&mouse_x, &mouse_y);
          zoom_out(img, mouse_x, mouse_y);
          break;
        case SDLK_r:
          img.mod = false;
          img.dest.x = 0;
          img.dest.y = 0;
          break;
        case SDLK_q:
          quit = true;
          break;
      }
    }
  }
  render(app, img);
  }
  clean_up(app);
}

std::vector<std::string> MinIMGView::load_from_wd(std::string dir) {
  std::vector<std::string> files;
  /* filesystem is really heavy on compile times (i haven't looked into it) 
   * maybe rewrite this function without using it. Leaving it for now as it works.
  */
  for(const auto & entry : std::filesystem::directory_iterator(dir))
    if(entry.path().string().find(".PNG") != std::string::npos)
      files.push_back(entry.path().string());

  return files;
}

SDL_Texture* MinIMGView::load_texture(std::string filename, Application &app) {
  return IMG_LoadTexture(app.renderer, filename.c_str());
}

std::string MinIMGView::get_wd(std::string path) {
  auto found = path.find_last_of("/\\");
  return(path.substr(0, found));
}