#pragma once

#include "Joint.h"
#include "Tokenizer.h"

class Skeleton {
public:
    Skeleton();
    ~Skeleton();

    bool Load(const char* filename);
    void Update();
    void Draw(const glm::mat4& viewProjMtx, GLuint shader);
    Joint* GetRoot() { return root; }

private:
    Joint* root;
};
