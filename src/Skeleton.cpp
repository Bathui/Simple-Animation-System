#include "Skeleton.h"

Skeleton::Skeleton() {
    root = nullptr;
}

Skeleton::~Skeleton() {
    if (root) delete root;
}

bool Skeleton::Load(const char* filename) {
    Tokenizer tokenizer;
    if (!tokenizer.Open(filename)) {
        return false;
    }

    char token[256];
    while(tokenizer.GetToken(token)) {
        if (strcmp(token, "balljoint") == 0) {
             root = new Joint();
             if (!root->Load(tokenizer)) {
                 delete root;
                 root = nullptr;
                 return false;
             }
             // Assumes only one root per file, or we break after finding it? 
             break;
        }
    }

    tokenizer.Close();
    return true;
}

void Skeleton::Update() {
    if (root) {
        root->Update(glm::mat4(1.0f));
    }
}

void Skeleton::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
    if (root) {
        root->Draw(viewProjMtx, shader);
    }
}
