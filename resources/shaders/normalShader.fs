#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform sampler2D specularMap;

uniform float heightScale;

uniform float constant;
uniform float linear;
uniform float quadratic;


uniform vec3 ambientL;
uniform vec3 diffuseL;
//uniform vec3 specularL;

uniform float factorL;
uniform float factorD;


vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
    float height =  texture(depthMap, texCoords).r;
    return texCoords - viewDir.xy * (height * heightScale);
}

void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoords = fs_in.TexCoords;

    texCoords = ParallaxMapping(fs_in.TexCoords,  viewDir);
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;


    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);



    // ambient
    vec3 ambient = ambientL * vec3(texture(diffuseMap, texCoords)) * factorL;


    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diffuseL * diff * vec3(texture(diffuseMap, texCoords)) * factorD;
    // specular




    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;


  // float distance = length(fs_in.TangentLightPos - fs_in.FragPos);
   //float attenuation = 2.0 / (constant + linear * distance + quadratic * (distance * distance));
   //specular = specular * attenuation;
   //diffuse = diffuse * attenuation;
   //ambient = ambient * attenuation;


    FragColor = vec4(ambient + diffuse + specular, 1.0);
}