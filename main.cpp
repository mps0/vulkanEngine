#include "window.hpp"
#include "vulkanBase.hpp"
#include "obj.hpp"
#include "model.hpp"

#include <iostream>
#include <algorithm>



int main() {


    Window window = Window();
    Scene scene = Scene();

    std::vector<Vertex> objVertices;
    std::vector<uint32_t> objIndices;
    obj(objVertices, objIndices);
       
    Model objModel = Model(objVertices, objIndices);
    scene.pushbackModel(&objModel);

    std::vector<Vertex> planeVertices = {
        {{-10.f, 0.f, 0.f}, {0.0f, 1.0f, 0.0f}, {0.f, 1.f, 0.f}},
        {{10.f, 0.f, 0.f}, {0.0f, 1.0f, 0.0f},  {0.f, 1.f, 0.f}},
        {{-10.f, 0.f, -20.f}, {0.0f, 1.0f, 0.0f},  {0.f, 1.f, 0.f}},
        {{10.f, 0.f, -20.f}, {0.0f, 1.0f, 0.0f},  {0.f, 1.f, 0.f}}

    };

    std::vector<uint32_t> planeIndices = {
        2, 0, 3, 3, 0, 1
    };
    Model plane = Model(planeVertices, planeIndices);
    scene.pushbackModel(&plane);

     
    VulkanBase base = VulkanBase(&window, &scene, true);
    base.createInstance();
    base.createSurface();
    base.createLogicalDevice();
    base.createSwapchain();
    base.createImageViews();
    base.createDepthBuffer();
    base.createUniformBuffers();
    base.createDescriptorSet();
    base.createRenderPass();
    base.createVertexBuffer();
    base.createIndexBuffer();
    base.createGraphicsPipeline();
    base.createFramebuffers();
    base.createCommandBuffers();
    base.createSyncObjects();




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
                                             base.cam->yaw(event.motion.xrel);
                                             base.cam->pitch(-event.motion.yrel);
                                             base.updateMVP();
                                         }
                                         else if (button & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
                                             base.cam->roll(event.motion.xrel);
                                             base.updateMVP();
                                         }
                                         break;
                                     }
                case SDL_KEYDOWN: {
                                      switch( event.key.keysym.sym )
                                      {
                                          case SDLK_w:
                                              base.cam->moveForward();
                                              base.updateMVP();
                                              break;

                                          case SDLK_s:
                                              base.cam->moveBackward();
                                              base.updateMVP();
                                              break;

                                          case SDLK_a:
                                              base.cam->moveLeft();
                                              base.updateMVP();
                                              break;

                                          case SDLK_d:
                                              base.cam->moveRight();
                                              base.updateMVP();
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

