#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "core.h"
#include "Tokenizer.h"
#include "Cube.h"

class Joint {
public:
    Joint();
    ~Joint();

    bool Load(Tokenizer& tokenizer);
    void Update(const glm::mat4& parentWorldMtx);
    void Draw(const glm::mat4& viewProjMtx, GLuint shader);

    // Tree traversal/Access
    void AddChild(Joint* child) { children.push_back(child); }

    const std::string& GetName() const { return name; }
    std::vector<Joint*>& GetChildren() { return children; }
    
    // Get pointers to pose components so ImGui can modify them directly
    float* GetPosePtr() { return &pose[0]; }
    
    glm::mat4 GetWorldMatrix () { return WorldMtx;}

    // Limit accessors
    glm::vec2 GetRotXLimit() const { return rotxlimit; }
    glm::vec2 GetRotYLimit() const { return rotylimit; }
    glm::vec2 GetRotZLimit() const { return rotzlimit; }

    void SetOffset(const glm::vec3& v) { offset = v; }
    void SetPose(const glm::vec3& v) { pose = v; }
    
private:
    // Hierarchical structure
    std::vector<Joint*> children;
    std::string name;

    // Joint properties
    glm::vec3 offset;
    glm::vec3 boxmin;
    glm::vec3 boxmax;
    glm::vec3 pose;      // Current DOF values (Euler angles)
    
    // Limits
    glm::vec2 rotxlimit;
    glm::vec2 rotylimit;
    glm::vec2 rotzlimit;

    // Matrices
    glm::mat4 WorldMtx;

    // Visual
    Cube* geometry; // The box to render
};
