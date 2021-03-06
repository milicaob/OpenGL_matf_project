#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;
uniform mat4 rot;

void main()
{
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));

    vec4 kon = vec4(R, 1.0) * rot;

    FragColor = vec4((texture(skybox, vec3(kon)).rgb * 0.65) + (vec3(0.7, 0.6, 0.15) * 0.2), 1.0);
}
