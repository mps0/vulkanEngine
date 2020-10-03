#ifndef WINDOW_HPP 
#define WINDOW_HPP 

#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>



#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 400


class Window {
    public:
        Window(); 
        ~Window();
        std::vector<const char*>getRequiredVulkanExtensions(bool print);
        SDL_Window* getWindow();

    private:
        SDL_Window* SDLwindow;



};















#endif
