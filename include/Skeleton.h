#pragma once
#include <vector>
#include "Joint.h"
#include "Tokenizer.h"


class Skeleton {
public:
    Skeleton();
    ~Skeleton();
    std::vector<Joint*> jointList;

    bool Load(const char* filename);
    void Update();
    void Draw(const glm::mat4& viewProjMtx, GLuint shader);
    Joint* GetRoot() { return root; }

    void BuildJointList(Joint* j);

private:
    Joint* root;

};
