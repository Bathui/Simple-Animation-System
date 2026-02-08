#version 330 core

in vec3 FragPos;
in vec3 FragNormal;

out vec4 color;

void main() {
    vec3 norm = normalize(FragNormal);
    
    // Key Light: Warm white from upper front-right
    vec3 lightPos1 = vec3(5.0, 8.0, 10.0);
    vec3 lightColor1 = vec3(1.0, 0.95, 0.9); // Slightly warm white
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec3 diffuse1 = diff1 * lightColor1 * 0.8;
    
    // Fill Light: Cool subtle light from left
    vec3 lightPos2 = vec3(-8.0, 3.0, 5.0);
    vec3 lightColor2 = vec3(0.5, 0.6, 0.8); // Cool fill
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec3 diffuse2 = diff2 * lightColor2 * 0.4;
    
    // Rim Light: Subtle backlight for separation from background
    vec3 lightPos3 = vec3(-3.0, 2.0, -8.0);
    vec3 lightColor3 = vec3(0.4, 0.5, 0.9); // Blue rim
    vec3 lightDir3 = normalize(lightPos3 - FragPos);
    float diff3 = max(dot(norm, lightDir3), 0.0);
    vec3 diffuse3 = diff3 * lightColor3 * 0.3;
    
    // Base object color (Light gray-beige)
    vec3 objectColor = vec3(0.85, 0.80, 0.75);
    
    // Ambient lighting
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    
    vec3 finalColor = (ambient + diffuse1 + diffuse2 + diffuse3) * objectColor;
    color = vec4(finalColor, 1.0);
}