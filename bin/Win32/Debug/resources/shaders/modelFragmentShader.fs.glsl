#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 myNormal;
in vec2 myTexPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 ambientIntensity;
uniform vec3 diffuseIntensity;
uniform vec3 specularIntensity;

void main()
{    
    float diff = max(dot(myNormal, -lightDir), 0.0);
    
    vec3 reflectDir = normalize(reflect(-lightDir, myNormal));
    vec3 viewDir = normalize(viewPos-FragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    
    vec3 ambient = ambientIntensity * vec3(texture(texture_diffuse1, myTexPos));
    vec3 diffuse = diffuseIntensity * diff * vec3(texture(texture_diffuse1, myTexPos));
    vec3 specular = specularIntensity * spec * vec3(texture(texture_specular1, myTexPos));
    FragColor = vec4(ambient+diffuse+specular,1.0f);
}