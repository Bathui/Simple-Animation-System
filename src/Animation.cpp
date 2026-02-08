#include "Animation.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cctype>

////////////////////////////////////////////////////////////////////////////////
// Animation
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Animation
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Animation
////////////////////////////////////////////////////////////////////////////////

Animation::Animation() {
    timeStart = 0.0f;
    timeEnd = 0.0f;
}

Animation::~Animation() {
}

bool Animation::Load(const char* filename) {
    Tokenizer tokenizer;
    if (!tokenizer.Open(filename)) {
        return false;
    }

    char token[256];
    tokenizer.GetToken(token); 
    if (strcmp(token, "animation") != 0) {
        printf("Error: Expected 'animation' tag\n");
        return false;
    }

    
    tokenizer.FindToken("{");

    while(true) {
        tokenizer.GetToken(token);
        if (strcmp(token, "}") == 0) break;

        if (strcmp(token, "range") == 0) {
            timeStart = tokenizer.GetFloat();
            timeEnd = tokenizer.GetFloat();
        }
        else if (strcmp(token, "numchannels") == 0) {
            int num = tokenizer.GetInt();
            channels.reserve(num);
        }
        else if (strcmp(token, "channel") == 0) {
            Channel ch;
            tokenizer.FindToken("{");
            while(true) {
                tokenizer.GetToken(token);
                if (strcmp(token, "}") == 0) break;

                if (strcmp(token, "extrapolate") == 0) {
                    tokenizer.GetToken(token); ch.extrapolateIn = token;
                    tokenizer.GetToken(token); ch.extrapolateOut = token;
                }
                else if (strcmp(token, "keys") == 0) {
                    int numKeys = tokenizer.GetInt();
                    ch.keyframes.resize(numKeys);
                    tokenizer.FindToken("{");
                    for(int i=0; i<numKeys; ++i) {
                        ch.keyframes[i].time = tokenizer.GetFloat();
                        ch.keyframes[i].value = tokenizer.GetFloat();
                        
                        tokenizer.GetToken(token);
                        if (isdigit(token[0]) || token[0] == '-') {
                             ch.keyframes[i].tangentInRule = "fixed";
                             ch.keyframes[i].tangentInValue = (float)atof(token);
                        } else {
                             ch.keyframes[i].tangentInRule = token;
                             ch.keyframes[i].tangentInValue = 0.0f; // placeholder
                        }

                        tokenizer.GetToken(token);
                        if (isdigit(token[0]) || token[0] == '-') {
                             ch.keyframes[i].tangentOutRule = "fixed";
                             ch.keyframes[i].tangentOutValue = (float)atof(token);
                        } else {
                             ch.keyframes[i].tangentOutRule = token;
                             ch.keyframes[i].tangentOutValue = 0.0f; // placeholder
                        }
                    }
                    tokenizer.FindToken("}");
                }
            }
            ch.Precompute();
            channels.push_back(ch);
        }
    }

    tokenizer.Close();
    return true;
}

void Animation::Evaluate(float time, Skeleton* skeleton) {
    if (!skeleton) return;

    // Apply root translation
    // Channels 0, 1, 2 are Root X, Y, Z translation
    if (channels.size() < 3) return;

    glm::vec3 rootTrans;
    rootTrans.x = channels[0].Evaluate(time);
    rootTrans.y = channels[1].Evaluate(time);
    rootTrans.z = channels[2].Evaluate(time);
    
    Joint* root = skeleton->GetRoot();
    if(root) {
        root->SetOffset(rootTrans);
    }

    // Iterate joints in DFS order
    // 3 channels per joint for rotation (X, Y, Z)
    // Starting channel index 3.
    
    int channelIdx = 3;
    if (root) {
        // Use a helper function to traverse DFS and apply rotations
        std::vector<Joint*> joints = skeleton->jointList;
        
        for (Joint* j : joints) {
            if (channelIdx + 3 > channels.size()) break;
            
            float rx = channels[channelIdx++].Evaluate(time);
            float ry = channels[channelIdx++].Evaluate(time);
            float rz = channels[channelIdx++].Evaluate(time);
            
            j->SetPose(glm::vec3(rx, ry, rz));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Channel
////////////////////////////////////////////////////////////////////////////////

float Channel::Evaluate(float time) {
    if (keyframes.empty()) return 0.0f;
    if (keyframes.size() == 1) return keyframes[0].value;

    // Handle time range and extrapolation
    float t = time;
    float firstTime = keyframes.front().time;
    float lastTime = keyframes.back().time;
    float duration = lastTime - firstTime;

    if (t < firstTime) {
        if (extrapolateIn == "constant") return keyframes.front().value;
        else if (extrapolateIn == "linear") {
             float slope = keyframes.front().tangentInValue;
             // slope is tangent (value/time).
             // If extrapolated linearly backwards:
             return keyframes.front().value + slope * (t - firstTime);
        }
        else if (extrapolateIn == "cycle") {
             float wrappedT = fmod(t - firstTime, duration);
             if (wrappedT < 0) wrappedT += duration;
             return Evaluate(firstTime + wrappedT);
        }
        else if (extrapolateIn == "cycle_offset") {
             float cycleCount = floor((t - firstTime) / duration);
             float wrappedT = t - firstTime - cycleCount * duration;
             float offset = (keyframes.back().value - keyframes.front().value) * cycleCount;
             return Evaluate(firstTime + wrappedT) + offset;
        }
        else if (extrapolateIn == "bounce") {
             float cycleCount = floor((t - firstTime) / duration);
             float wrappedT = t - firstTime - cycleCount * duration;
             if ((int)std::abs(cycleCount) % 2 != 0) { // odd, bouncing back
                 return Evaluate(lastTime - wrappedT);
             } else {
                 return Evaluate(firstTime + wrappedT);
             }
        }
        else return keyframes.front().value; // Default constant
    }
    else if (t > lastTime) {
         if (extrapolateOut == "constant") return keyframes.back().value;
         else if (extrapolateOut == "linear") {
             float slope = keyframes.back().tangentOutValue; // Out tangent of last key
             return keyframes.back().value + slope * (t - lastTime);
         }
         else if (extrapolateOut == "cycle") {
             float wrappedT = fmod(t - firstTime, duration);
             if (wrappedT < 0) wrappedT += duration;
             return Evaluate(firstTime + wrappedT);
         }
         else if (extrapolateOut == "cycle_offset") {
             float cycleCount = floor((t - firstTime) / duration);
             float wrappedT = t - firstTime - cycleCount * duration;
             float offset = (keyframes.back().value - keyframes.front().value) * cycleCount;
             return Evaluate(firstTime + wrappedT) + offset;
         }
         else if (extrapolateOut == "bounce") {
             float cycleCount = floor((t - firstTime) / duration); 
             float wrappedT = t - firstTime - cycleCount * duration;
             // If cycleCount is even, forward. If odd, backward.
             if ((int)cycleCount % 2 != 0) { // odd, backwards
                 // e.g. 1..2. cycle=1. wrappedT = t-first-dur. 
                 // we want evaluate(last - wrappedT).
                 return Evaluate(lastTime - wrappedT);
             } else {
                 return Evaluate(firstTime + wrappedT);
             }
         }
         else return keyframes.back().value; // Default constant
    }

    // Find segment
    // base case fall into this loop
    for (size_t i = 0; i < keyframes.size() - 1; ++i) {
        if (t >= keyframes[i].time && t <= keyframes[i+1].time) {
            return EvaluateSegment(i, t);
        }
    }
    return keyframes.back().value;
}

float Channel::EvaluateSegment(int i, float t) {
    Keyframe& p0 = keyframes[i];
    Keyframe& p1 = keyframes[i+1];
    
    // Normalize t to 0..1 range
    float u = (t - p0.time) / (p1.time - p0.time);
    
    // Cubic Hermite Spline
    
    float dt = p1.time - p0.time;
    float m0 = p0.tangentOutValue * dt;
    float m1 = p1.tangentInValue * dt;
    
    float u2 = u * u;
    float u3 = u2 * u;
    
    return (2*u3 - 3*u2 + 1) * p0.value +
           (u3 - 2*u2 + u) * m0 +
           (-2*u3 + 3*u2) * p1.value +
           (u3 - u2) * m1;
}

void Channel::Precompute() {
    if (keyframes.empty()) return;

    // Calculate tangents for each keyframe based on rules
    for (size_t i = 0; i < keyframes.size(); ++i) {
        Keyframe& key = keyframes[i];
        
        // Tangent In
        if (key.tangentInRule == "flat") {
            key.tangentInValue = 0.0f;
        } else if (key.tangentInRule == "linear") {
            // Slope to previous key
            if (i > 0) {
                Keyframe& prev = keyframes[i-1];
                key.tangentInValue = (key.value - prev.value) / (key.time - prev.time);
            } else {
                key.tangentInValue = 0.0f; // Start
            }
        } else if (key.tangentInRule == "smooth") {
            // "smooth" usually implies slope based on prev and next.
            if (i > 0 && i < keyframes.size() - 1) {
                Keyframe& prev = keyframes[i-1];
                Keyframe& next = keyframes[i+1];
                key.tangentInValue = (next.value - prev.value) / (next.time - prev.time);
            } else if (i > 0) {
                Keyframe& prev = keyframes[i-1];
                key.tangentInValue = (key.value - prev.value) / (key.time - prev.time);
            } else {
                key.tangentInValue = 0.0f;
            }
        }
        // Fixed is already set in Load

        // Tangent Out
        if (key.tangentOutRule == "flat") {
            key.tangentOutValue = 0.0f;
        } else if (key.tangentOutRule == "linear") {
            // Slope to next key
            if (i < keyframes.size() - 1) {
                Keyframe& next = keyframes[i+1];
                key.tangentOutValue = (next.value - key.value) / (next.time - key.time);
            } else {
                key.tangentOutValue = 0.0f;
            }
        } else if (key.tangentOutRule == "smooth") {
             if (i > 0 && i < keyframes.size() - 1) {
                Keyframe& prev = keyframes[i-1];
                Keyframe& next = keyframes[i+1];
                key.tangentOutValue = (next.value - prev.value) / (next.time - prev.time);
            } else if (i < keyframes.size() - 1) {
                Keyframe& next = keyframes[i+1];
                key.tangentOutValue = (next.value - key.value) / (next.time - key.time);
            } else {
                key.tangentOutValue = 0.0f;
            }
        }
        // Fixed is already set
    }
}
