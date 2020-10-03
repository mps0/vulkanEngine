#include <stdexcept>
#include <iostream>

#include "window.hpp"


Window::Window() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS); 
    SDLwindow =SDL_CreateWindow("vulkan test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_VULKAN);

}

std::vector<const char*> Window::getRequiredVulkanExtensions(bool print) {

    unsigned int extensionCount;
    SDL_Vulkan_GetInstanceExtensions(SDLwindow, &extensionCount, nullptr);
    std::vector<const char*> extensions(extensionCount);
    if (!SDL_Vulkan_GetInstanceExtensions(SDLwindow, &extensionCount, extensions.data())) {
     throw std::runtime_error("failed to get SDL_Vulkan extensions.");
    };

    if (print) {
    std::cout << "SDL_Vulkan required extensions:\n";
    for(const char* extension : extensions) {
        std::cout << '\t' << extension << '\n';
    }
    }

    return extensions;
}

SDL_Window* Window::getWindow() {
    return  SDLwindow;
}

Window::~Window() {
    SDL_DestroyWindow(SDLwindow);
    SDL_Quit();
}
