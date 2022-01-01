#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform bool grayEffect;
uniform sampler2DMS screenTex;

void main()
{
    ivec2 viewPortDim = ivec2(1280, 720);
    ivec2 coord = ivec2(viewPortDim*TexCoords);


    vec3 sample0=texelFetch(screenTex, coord, 0).rgb;
    vec3 sample1=texelFetch(screenTex, coord, 1).rgb;
    vec3 sample2=texelFetch(screenTex, coord, 2).rgb;
    vec3 sample3=texelFetch(screenTex, coord, 3).rgb;


    vec3 col = 0.25*(sample0+sample1+sample2+sample3);

    if(grayEffect){
        float grayscale = 0.2126 * col.r + 0.7162 * col.g + 0.0722 * col.b;
        FragColor = vec4(vec3(grayscale), 1.0);
    }
    else {
        FragColor = vec4(vec3(col), 1.0);
    }
}