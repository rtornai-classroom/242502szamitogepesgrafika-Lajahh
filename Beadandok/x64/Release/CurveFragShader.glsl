#version 400

uniform vec3 curveColor;
out vec4 outColor;

void main() {
    outColor = vec4(curveColor, 1.0);
}