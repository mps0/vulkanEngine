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
    base.createDepthBuffer();
    base.createUniformBuffer();
    base.createDescriptorSet();
    base.createRenderPass();
    base.createVertexBuffer();
    base.createGraphicsPipeline();
    base.createFramebuffers();
    base.createCommandBuffers();
    base.createSyncObjects();

    getchar();

    SDL_Event event;
    bool run = true;
    while(run) {
        while( SDL_PollEvent( &event ) != 0 ){
            switch(event.type) {
                case SDL_QUIT: {
                                   run = false;
                                   break;
                               }

                case SDL_MOUSEMOTION:{
                                         unsigned int button = SDL_GetMouseState(NULL, NULL);
                                         if (button & SDL_BUTTON(SDL_BUTTON_LEFT)) {
                                             //cam->rotate(event.motion.xrel, -event.motion.yrel);
                                         }
                                         else if (button & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                                             //cam->roll(event.motion.xrel);
                                         }
                                         break;
                                     }
                case SDL_KEYDOWN: {
                                      switch( event.key.keysym.sym )
                                      {
                                          case SDLK_w:
                                              base.cam->moveForward();

                                              break;

                                          case SDLK_s:
                                              break;

                                          case SDLK_a:
                                              break;

                                          case SDLK_d:
                                              break;


                                  }
            }
        }
        }
        base.draw();
    }
    base.cleanUp();
    SDL_Quit();
}

