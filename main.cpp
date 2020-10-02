#include "window.hpp"
#include "vulkanBase.hpp"

#include <iostream>


int main() {

    Window window = Window();
    VulkanBase base = VulkanBase(&window, true);
    base.createInstance();
    base.createSurface();
    base.createLogicalDevice();
    base.createSwapchain();
    base.createImageViews();
    base.createCommandBuffers();
    base.createDepthBuffer();

    SDL_Event event;
    bool run = true;
    while(run) {
        while( SDL_PollEvent( &event ) != 0 ){
            switch(event.type) {
                case SDL_QUIT: {
                                   run = false;
                                   break;
                               }

            }
        }
    }
    base.cleanUp();
    SDL_Quit();
}
