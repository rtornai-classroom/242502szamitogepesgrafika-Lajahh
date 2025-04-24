#version 400 core
layout (vertices = 32) out; // Set to maximum expected points

void main() {
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1.0;
        gl_TessLevelOuter[1] = 64.0;
    }
    // Only process points that exist
    if (gl_InvocationID < gl_PatchVerticesIn) {
        gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    }
}