#version 330 core
in vec3 vNormal;
in vec2 vUV;

uniform vec3 uColor;

out vec4 FragColor;

void main() {
    // Blinn-Phong lighting with color and UV-based variation
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));

    // Ambient contribution
    vec3 ambient = 0.4 * uColor;

    // Diffuse contribution
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uColor * diff;

    // UV-based shader demo effect
    vec3 uvEffect = vec3(vUV.x, vUV.y, 1.0 - vUV.x * vUV.y) * 0.2;
    
    vec3 result = ambient + diffuse * 0.7 + uvEffect;
    FragColor = vec4(result, 1.0);
}
