#version 330 core
out vec4 FragColor;
  
in vec3 myCol;
in vec2 myTexPos;

uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
    FragColor = mix(texture(tex1, myTexPos), texture(tex2, myTexPos), 0.5) * vec4(myCol, 1.0);
}