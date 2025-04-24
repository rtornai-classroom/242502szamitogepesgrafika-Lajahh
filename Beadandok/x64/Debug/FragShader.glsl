#version 330 core
out vec4 FragColor;
uniform vec2 circCenter;
uniform float circRadius;
uniform vec3 innerColor;
uniform vec3 outerColor;
void main() {
    vec2 fragCoord = gl_FragCoord.xy;
    float dx = fragCoord.x - circCenter.x;
    float dy = fragCoord.y - circCenter.y;
    float dist = sqrt(dx * dx + dy * dy);
    if (dist > circRadius) {
        discard;
    }
    float t = dist / circRadius;
    FragColor = vec4(mix(innerColor, outerColor, t), 1.0);
}