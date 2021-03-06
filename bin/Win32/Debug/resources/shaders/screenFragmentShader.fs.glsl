#version 330 core
out vec4 FragColor;

in vec2 myTexPos;

uniform sampler2DMS screenTexture;
uniform bool shouldAA;
uniform bool shouldGrayscale;
uniform ivec2 viewPortDim;

void main()
{
    vec3 col;
    ivec2 coords = ivec2(viewPortDim * myTexPos);
    if(shouldAA){
        vec3 sample0 = texelFetch(screenTexture,coords,0).rgb;
        vec3 sample1 = texelFetch(screenTexture,coords,1).rgb;
        vec3 sample2 = texelFetch(screenTexture,coords,2).rgb;
        vec3 sample3 = texelFetch(screenTexture,coords,3).rgb;
        vec3 sample4 = texelFetch(screenTexture,coords,4).rgb;
        vec3 sample5 = texelFetch(screenTexture,coords,5).rgb;
        vec3 sample6 = texelFetch(screenTexture,coords,6).rgb;
        vec3 sample7 = texelFetch(screenTexture,coords,7).rgb;
        col = 0.125 * (sample0 + sample1 + sample2 + sample3 + sample4 + sample5 + sample6 + sample7);
    } else {
        
        vec3 sample0 = texelFetch(screenTexture,coords,0).rgb;
        col = sample0;
    }
    if(shouldGrayscale){
        float grayscale = 0.21 * col.r + 0.7 * col.g + 0.07 * col.b;
        col = vec3(grayscale);
    }
    FragColor = vec4(col, 1.0);
} 