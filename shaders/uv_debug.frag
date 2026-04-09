#version 330 core
in vec3 vNormal;
in vec2 vUV;

out vec4 FragColor;

void main() {
    // Visualize UV coordinates directly: R = U, G = V, B = 0.0
    FragColor = vec4(vUV.x, vUV.y, 0.0, 1.0);
}
