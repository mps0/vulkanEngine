#ifndef MODEL_HPP
#define MODEL_HPP

#include "glm/glm.hpp"
#include "vertex.hpp"

#include <vector>

class Model {
    public:
        Model(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::mat4 model = glm::mat4(1.f)); 
        std::vector<Vertex> getVerts();
        std::vector<uint32_t> getInds();


private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    glm::mat4 model;

};

class Scene {
    public:
        Scene();
        std::vector<Vertex>* getVerts();
        std::vector<uint32_t>* getInds();

        void pushbackModel(Model* pModel);
        uint32_t getModelCount();


        std::vector<uint32_t> modelMarkers;

    private:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;


        uint32_t modelCount = 0;
};


#endif
