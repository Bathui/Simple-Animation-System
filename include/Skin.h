#pragma once
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Tokenizer.h"
#include "Skeleton.h"

class Skin {
public:
    Skin();
    ~Skin();

    bool Load(const char* filename);
    void Update(Skeleton* skeleton); // Computes bone matrices
    void Draw(const glm::mat4& viewProjMtx, GLuint shader);

private:
    // CPU Data
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::mat4> bindings; // Inverse bind matrices
    std::vector<unsigned int> indices;

    // GPU-friendly Skin Weights
    // We limit to 4 weights per vertex for the shader
    struct VertexBoneData {
        glm::vec4 weights;
        glm::ivec4 ids; 
    };
    std::vector<VertexBoneData> skinWeights;

    // Matrices to send to GPU
    std::vector<glm::mat4> skinningMatrices;

    // GL buffers
    GLuint VAO;
    GLuint VBO_pos, VBO_norm, VBO_weights, VBO_ids, EBO;
};