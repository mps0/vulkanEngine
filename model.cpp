#include "model.hpp"

#include <algorithm>
#include <iostream>


Model::Model(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::mat4 model) : vertices(vertices), indices(indices), model(model) {} 


std::vector<Vertex>* Model::getVerts() {
    return &vertices;
}

std::vector<uint32_t>* Model::getInds() {
    return &indices;
}





Scene::Scene() {}

void Scene::pushbackModel(Model* pModel) {

    std::vector<Vertex>* pVerts = pModel->getVerts();
    std::vector<uint32_t>* pInds = pModel->getInds();

    std::transform(pInds->begin(), pInds->end(), pInds->begin(), [this](uint32_t i) {return i + offset;});

    vertices.insert(vertices.end(), pVerts->begin(), pVerts->end());
    indices.insert(indices.end(), pInds->begin(), pInds->end());

    offset = vertices.size();
    modelCount++;
    modelMarkers.push_back(indices.size());
}

std::vector<Vertex>* Scene::getVerts() {
    return &vertices;
}

std::vector<uint32_t>* Scene::getInds() {
    return &indices;
}

uint32_t Scene::getModelCount() {
    return modelCount;
}




