#include "Joint.h"

Joint::Joint() {
    // defaults
    offset = glm::vec3(0.0f);
    boxmin = glm::vec3(-0.1f);
    boxmax = glm::vec3(0.1f);
    pose = glm::vec3(0.0f); // Radian values
    
    float pi = 3.1415926535f;
    // Large limits by default
    rotxlimit = glm::vec2(-pi, pi); // Default to -180 to 180 degrees
    rotylimit = glm::vec2(-pi, pi);
    rotzlimit = glm::vec2(-pi, pi);

    WorldMtx = glm::mat4(1.0f);
    geometry = nullptr;
}

Joint::~Joint() {
    if (geometry) delete geometry;
    for (auto c : children) {
        delete c;
    }
}

bool Joint::Load(Tokenizer& tokenizer) {
    char token[256];
    
    // We expect the 'balljoint' token was already consumed by the caller,
    // and followed by the name and then '{'.
    // Or, maybe the caller just saw 'balljoint', so we need to get name and '{'.
    tokenizer.GetToken(token);
    name = token;

    tokenizer.GetToken(token);
    if (strcmp(token, "{") != 0) {
        // If tokenizer didn't split '{' from name? The sample skel has spaces.
        // If it failed, it might be an error.
        std::cerr << "Expected '{' after joint name " << name << ", got " << token << std::endl;
        return false;
    }

    while (true) {
        tokenizer.GetToken(token);
        if (strcmp(token, "}") == 0) {
            break; // End of this joint
        }
        else if (strcmp(token, "offset") == 0) {
            offset.x = tokenizer.GetFloat();
            offset.y = tokenizer.GetFloat();
            offset.z = tokenizer.GetFloat();
        }
        else if (strcmp(token, "boxmin") == 0) {
            boxmin.x = tokenizer.GetFloat();
            boxmin.y = tokenizer.GetFloat();
            boxmin.z = tokenizer.GetFloat();
        }
        else if (strcmp(token, "boxmax") == 0) {
            boxmax.x = tokenizer.GetFloat();
            boxmax.y = tokenizer.GetFloat();
            boxmax.z = tokenizer.GetFloat();
        }
        else if (strcmp(token, "rotxlimit") == 0) {
            rotxlimit.x = tokenizer.GetFloat();
            rotxlimit.y = tokenizer.GetFloat();
        }
        else if (strcmp(token, "rotylimit") == 0) {
            rotylimit.x = tokenizer.GetFloat();
            rotylimit.y = tokenizer.GetFloat();
        }
        else if (strcmp(token, "rotzlimit") == 0) {
            rotzlimit.x = tokenizer.GetFloat();
            rotzlimit.y = tokenizer.GetFloat();
        }
        else if (strcmp(token, "pose") == 0) {
            pose.x = tokenizer.GetFloat();
            pose.y = tokenizer.GetFloat();
            pose.z = tokenizer.GetFloat();
        }
        else if (strcmp(token, "balljoint") == 0) {
            Joint* child = new Joint();
            if(!child->Load(tokenizer)) return false;
            children.push_back(child);
        }
        else {
            // Unknown token? Skip or error.
            std::cerr << "Unknown token: " << token << " in joint " << name << std::endl;
          
        }
    }

    // Initialize geometry
    geometry = new Cube(boxmin, boxmax);

    return true;
}

void Joint::Update(const glm::mat4& parentWorldMtx) {
    // 1. Clamp pose to limits
    glm::vec3 curPose = pose;
    curPose.x = glm::clamp(curPose.x, rotxlimit.x, rotxlimit.y);
    curPose.y = glm::clamp(curPose.y, rotylimit.x, rotylimit.y);
    curPose.z = glm::clamp(curPose.z, rotzlimit.x, rotzlimit.y);

    // 2. Compute Local Matrix
    // Order: Translate(offset) * RotateZ * RotateY * RotateX
    glm::mat4 local = glm::translate(glm::mat4(1.0f), offset);
    local = glm::rotate(local, curPose.z, glm::vec3(0, 0, 1));
    local = glm::rotate(local, curPose.y, glm::vec3(0, 1, 0));
    local = glm::rotate(local, curPose.x, glm::vec3(1, 0, 0));

    // 3. Compute World Matrix
    WorldMtx = parentWorldMtx * local;

    // 4. Update children
    for (auto c : children) {
        c->Update(WorldMtx);
    }
}

void Joint::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
    if (geometry) {
        geometry->setModel(WorldMtx);
        geometry->draw(viewProjMtx, shader);
    }

    for (auto c : children) {
        c->Draw(viewProjMtx, shader);
    }
}
