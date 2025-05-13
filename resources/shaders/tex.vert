#version 460 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uM_m;
uniform mat4 uV_m;
uniform mat4 uP_m;

uniform vec3 pointLightPositions[3];

out VS_OUT {
    vec3 N;
    vec3 V;
    vec2 texCoord;
    vec3 FragPos_world;
    vec3 L_point[3];
} vs_out;

void main() {
    vec4 worldPos = uM_m * vec4(aPosition, 1.0);
    vec4 viewPos = uV_m * worldPos;

    vs_out.FragPos_world = worldPos.xyz;
    vs_out.N = normalize(mat3(uM_m) * aNormal);
    vs_out.V = normalize(vec3(uV_m * worldPos));
    vs_out.texCoord = aTexCoord;

    for (int i = 0; i < 3; ++i) {
        vs_out.L_point[i] = pointLightPositions[i] - worldPos.xyz;
    }

    gl_Position = uP_m * viewPos;
}
