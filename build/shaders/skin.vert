#version 330 core

// Input vertex data
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
// Maximum 4 bone influences per vertex is standard
layout(location = 2) in vec4 in_BoneWeights;
layout(location = 3) in ivec4 in_BoneIndices;


// Uniforms
uniform mat4 viewProj;
uniform mat4 model; // usually Identity for the skin itself

// Array of matrices: (WorldMatrix * InverseBindMatrix) for every joint
const int MAX_BONES = 100;
uniform mat4 boneMatrices[MAX_BONES];

// Outputs to Fragment Shader
out vec3 FragPos;
out vec3 FragNormal;

void main() {
    // 1. Calculate Skinning Matrix
    // Sum of (Weight * BoneMatrix)
    mat4 skinMatrix = 
        in_BoneWeights.x * boneMatrices[in_BoneIndices.x] +
        in_BoneWeights.y * boneMatrices[in_BoneIndices.y] +
        in_BoneWeights.z * boneMatrices[in_BoneIndices.z] +
        in_BoneWeights.w * boneMatrices[in_BoneIndices.w];

    // 2. Transform Position
    // Apply skin matrix first (local deformation), then viewProj
    vec4 skinnedPos = skinMatrix * vec4(in_Position, 1.0);
    gl_Position = viewProj * model * skinnedPos;
    
    // Pass world position to fragment shader
    FragPos = vec3(model * skinnedPos);

    // 3. Transform Normal
    // Normals must use the inverse transpose of the transformation matrix
    mat3 normalMatrix = transpose(inverse(mat3(skinMatrix)));
    vec3 worldNormal = mat3(model) * normalMatrix * in_Normal;
    FragNormal = normalize(worldNormal);
}