#include "model.hpp"

#include <algorithm>
#include <iostream>


Model::Model(std::vector<Vertex> vertices, std::vector<uint32_t> indices, glm::mat4 model) : vertices(vertices), indices(indices), model(model) {} 


std::vector<Vertex> Model::getVerts() {
    return vertices;
}

std::vector<uint32_t> Model::getInds() {
    return indices;
}



Scene::Scene() {}

void Scene::pushbackModel(Model* pModel) {

    std::vector<Vertex> verts = pModel->getVerts();
    std::vector<uint32_t> inds = pModel->getInds();

    std::transform(inds.begin(), inds.end(), inds.begin(), [this](uint32_t i) {return i + vertices.size();});

    vertices.insert(vertices.end(), verts.begin(), verts.end());
    indices.insert(indices.end(), inds.begin(), inds.end());

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




