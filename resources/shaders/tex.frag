#version 460 core

struct PointLight {
    vec3 ambient, diffuse, specular;
    float constant, linear, quadratic;
};

uniform PointLight pointLights[3];

uniform vec3 directionalLight_direction;
uniform vec3 directionalLight_ambient;
uniform vec3 directionalLight_diffuse;
uniform vec3 directionalLight_specular;

uniform vec3 spotLight_position;
uniform vec3 spotLight_direction;
uniform float spotLight_constant;
uniform float spotLight_linear;
uniform float spotLight_quadratic;
uniform float spotLight_cutoff;
uniform float spotLight_outerCutoff;

uniform float shininess;
uniform sampler2D uTexture;

in VS_OUT {
    vec3 N;
    vec3 V;
    vec2 texCoord;
    vec3 FragPos_world;
    vec3 L_point[3];
} fs_in;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(uTexture, fs_in.texCoord);
    //if (texColor.a < 0.1) discard;

    vec3 N = normalize(fs_in.N);
    vec3 V = normalize(fs_in.V);
    vec3 result = vec3(0.0);

    // === Directional light (Phong) ===
    vec3 L_dir = normalize(-directionalLight_direction);
    vec3 R_dir = reflect(-L_dir, N);
    float diff_dir = max(dot(N, L_dir), 0.0);
    float spec_dir = pow(max(dot(R_dir, V), 0.0), shininess);
    result += (
        directionalLight_ambient *2.0 +
        directionalLight_diffuse * diff_dir * 4.0+
        directionalLight_specular * spec_dir
    ) * texColor.rgb;

    // === Point Lights (soft diffuse, no specular) ===
    for (int i = 0; i < 3; ++i) {
        vec3 L = normalize(fs_in.L_point[i]);
        float dist = length(fs_in.L_point[i]);
        float attenuation = 1.0 / (pointLights[i].constant +
                                   pointLights[i].linear * dist +
                                   pointLights[i].quadratic * dist * dist);

        float diff = max(dot(N, L), 0.2); // min 0.2 to avoid full black
        vec3 diffuse = pointLights[i].diffuse * diff * texColor.rgb;

        result += attenuation * diffuse;
    }

    // === Spotlight (pure cone) ===
    vec3 L_spot = spotLight_position - fs_in.FragPos_world;
    float dist_spot = length(L_spot);
    vec3 L_dir_spot = normalize(L_spot);
    float theta = dot(L_dir_spot, normalize(-spotLight_direction));
    float epsilon = spotLight_cutoff - spotLight_outerCutoff;
    float intensity = clamp((theta - spotLight_outerCutoff) / epsilon, 0.0, 1.0);
    float attenuation_spot = 1.0 / (spotLight_constant +
                                    spotLight_linear * dist_spot +
                                    spotLight_quadratic * dist_spot * dist_spot);

    // Intensity-only spotlight — no normal used
    vec3 spotlight = vec3(0.4) * intensity * attenuation_spot; // multiplier for brightness
    result += spotlight;

    FragColor = vec4(min(result, vec3(1.0)), texColor.a); // clamp to avoid overbright
}
