#version 330 core
out vec4 FragColor;

struct PointLight {
    vec3 position;

    vec3 specular;
    vec3 diffuse;
    vec3 ambient;

    float constant;
    float linear;
    float quadratic;
};


uniform sampler2D diffTex;
uniform sampler2D specTex;


in vec2 TexCoords;
in vec3 FragPos;

uniform PointLight pointLight;
uniform vec3 viewPosition;




void main()
{

    vec4 res = texture(diffTex, TexCoords);


    FragColor = res;
}



