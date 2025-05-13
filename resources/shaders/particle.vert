#version 460 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uV_m;
uniform mat4 uP_m;

void main() {
    gl_Position = uP_m * uV_m * vec4(aPosition, 1.0);
    gl_PointSize = 10.0; // You can make this dynamic if needed
}
