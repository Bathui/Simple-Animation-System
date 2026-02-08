#include "Skin.h"

Skin::Skin() {
    VAO = 0;
}

Skin::~Skin() {
    glDeleteBuffers(1, &VBO_pos);
    glDeleteBuffers(1, &VBO_norm);
    glDeleteBuffers(1, &VBO_weights);
    glDeleteBuffers(1, &VBO_ids);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

bool Skin::Load(const char* filename) {
    Tokenizer tokenizer;
    if (!tokenizer.Open(filename)) return false;

    char token[256];
    
    // READ POSITIONS
    tokenizer.GetToken(token); // "positions"
    int numVerts = tokenizer.GetInt();
    tokenizer.GetToken(token); // "{"
    positions.resize(numVerts);
    skinWeights.resize(numVerts); // Resize weights here too
    for(int i=0; i<numVerts; i++) {
        positions[i].x = tokenizer.GetFloat();
        positions[i].y = tokenizer.GetFloat();
        positions[i].z = tokenizer.GetFloat();
        
        // Initialize weights to 0
        skinWeights[i].weights = glm::vec4(0.0f);
        skinWeights[i].ids = glm::ivec4(0);
    }
    tokenizer.GetToken(token); // "}"

    // READ NORMALS
    tokenizer.GetToken(token); // "normals"
    int numNorms = tokenizer.GetInt();
    tokenizer.GetToken(token); // "{"
    normals.resize(numNorms);
    for(int i=0; i<numNorms; i++) {
        normals[i].x = tokenizer.GetFloat();
        normals[i].y = tokenizer.GetFloat();
        normals[i].z = tokenizer.GetFloat();
    }
    tokenizer.GetToken(token); // "}"

    // READ SKIN WEIGHTS
    tokenizer.GetToken(token); // "skinweights"
    int numWeights = tokenizer.GetInt(); // Should match numVerts
    tokenizer.GetToken(token); // "{"
    for(int i=0; i<numWeights; i++) {
        int numAttachments = tokenizer.GetInt();
        float totalWeight = 0.0f;

        for(int j=0; j<numAttachments; j++) {
            int jointID = tokenizer.GetInt();
            float weight = tokenizer.GetFloat();
            
            // Store only top 4 weights for GPU (GLSL limitation simplifier)
            if(j < 4) {
                skinWeights[i].ids[j] = jointID;
                skinWeights[i].weights[j] = weight;
                totalWeight += weight;
            }
        }
        
        // Normalize weights after reading all attachments
        if (totalWeight > 0.0f) {
            skinWeights[i].weights /= totalWeight; 
        }
    }
    tokenizer.GetToken(token); // "}"

    // READ TRIANGLES
    tokenizer.GetToken(token); // "triangles"
    int numTris = tokenizer.GetInt();
    tokenizer.GetToken(token); // "{"
    for(int i=0; i<numTris; i++) {
        indices.push_back(tokenizer.GetInt());
        indices.push_back(tokenizer.GetInt());
        indices.push_back(tokenizer.GetInt());
    }
    tokenizer.GetToken(token); // "}"

    // READ BINDINGS
    tokenizer.GetToken(token); // "bindings"
    int numJoints = tokenizer.GetInt();
    tokenizer.GetToken(token); // "{"
    bindings.resize(numJoints);
    for(int i=0; i<numJoints; i++) {
        tokenizer.GetToken(token); // "matrix"
        tokenizer.GetToken(token); // "{"
        // Read 4x3 or 4x4. The sample shows ax, ay, az... 
        // usually it is stored column-major or row-major. 
        // Assuming standard mathematical notation inputs:
        glm::mat4& m = bindings[i];
        
        // The file format image shows: ax ay az / bx by bz / cx cy cz / dx dy dz
        // This looks like rows. glm is column-major.
        float ax = tokenizer.GetFloat(); float ay = tokenizer.GetFloat(); float az = tokenizer.GetFloat();
        float bx = tokenizer.GetFloat(); float by = tokenizer.GetFloat(); float bz = tokenizer.GetFloat();
        float cx = tokenizer.GetFloat(); float cy = tokenizer.GetFloat(); float cz = tokenizer.GetFloat();
        float dx = tokenizer.GetFloat(); float dy = tokenizer.GetFloat(); float dz = tokenizer.GetFloat();
        
        // Fill GLM matrix (Column-Major)
        // Rows are:
        // ax ay az dx
        // bx by bz dy
        // cx cy cz dz
        // 0  0  0  1
        
        m[0] = glm::vec4(ax, bx, cx, 0.0f);
        m[1] = glm::vec4(ay, by, cy, 0.0f);
        m[2] = glm::vec4(az, bz, cz, 0.0f);
        m[3] = glm::vec4(dx, dy, dz, 1.0f);
        
        tokenizer.GetToken(token); // "}"
    }
    tokenizer.GetToken(token); // "}"
    tokenizer.Close();

    // --- SETUP BUFFERS ---
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Positions (Loc 0)
    glGenBuffers(1, &VBO_pos);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Normals (Loc 1)
    glGenBuffers(1, &VBO_norm);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Weights (Loc 2)
    glGenBuffers(1, &VBO_weights);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_weights);
    glBufferData(GL_ARRAY_BUFFER, skinWeights.size() * sizeof(VertexBoneData), skinWeights.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    // Offset 0 of struct
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (void*)0);

    // Bone Indices (Loc 3)
    // Note: use glVertexAttribIPointer for Integers!
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData), (void*)sizeof(glm::vec4));

    // EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    
    // Initialize skinning matrices to identity for bind pose rendering
    // (when no skeleton is loaded)
    skinningMatrices.resize(bindings.size());
    for(size_t i = 0; i < skinningMatrices.size(); i++) {
        skinningMatrices[i] = glm::mat4(1.0f);
    }
    
    return true;
}

void Skin::Update(Skeleton* skeleton) {
    // If no skeleton, keep identity matrices (bind pose)
    if (!skeleton) return;
    
    // 1. Get world matrices from skeleton
    // This assumes the skeleton's jointList order matches the binding matrices order
    // which is standard for this project type.
    const auto& joints = skeleton->jointList; // You might need to add a getter to Skeleton.h
    
    skinningMatrices.resize(bindings.size());

    for(size_t i=0; i < bindings.size(); i++) {
        if(i < joints.size()) {
            glm::mat4 worldMtx = joints[i]->GetWorldMatrix();
            glm::mat4 inv_bindingMtx = glm::inverse(bindings[i]);
            
            // Skin Matrix = World * InverseBind
            // If bindings are stored as bind pose (not inverse), we need to invert them
            skinningMatrices[i] = worldMtx * inv_bindingMtx;
        } else {
            skinningMatrices[i] = glm::mat4(1.0f);
        }
    }
}

void Skin::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
    glUseProgram(shader);
    
    // Pass ViewProj
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewProj"), 1, GL_FALSE, &viewProjMtx[0][0]);
    
    // Pass Model (Identity)
    glm::mat4 model(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, &model[0][0]);

    // Pass Bone Matrices Array
    GLint boneLoc = glGetUniformLocation(shader, "boneMatrices");
    glUniformMatrix4fv(boneLoc, skinningMatrices.size(), GL_FALSE, &skinningMatrices[0][0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}