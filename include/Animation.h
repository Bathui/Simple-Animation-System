#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "core.h"
#include "Tokenizer.h"
#include "Skeleton.h"

class Keyframe {
public:
    float time;
    float value;
    std::string tangentInRule;
    float tangentInValue;
    std::string tangentOutRule;
    float tangentOutValue;
    float A, B, C, D; // Cubic coefficients
};

class Channel {
public:
    std::string extrapolateIn;
    std::string extrapolateOut;
    std::vector<Keyframe> keyframes;

    float Evaluate(float time);
    void Precompute(); // Calculate tangents and coefficients

private:
    float EvaluateSegment(int i, float t);
    
};

class Animation {
public:
    Animation();
    ~Animation();

    bool Load(const char* filename);
    void Evaluate(float time, Skeleton* skeleton);

    float GetStartTime() const { return timeStart; }
    float GetEndTime() const { return timeEnd; }

private:
    float timeStart;
    float timeEnd;
    std::vector<Channel> channels;
};
