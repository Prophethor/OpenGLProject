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
    float diff = max(dot(myNormal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, myNormal);
    vec3 viewDir = normalize(viewPos-FragPos);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
    // combine results
    vec3 ambient = ambientIntensity * texture(texture_diffuse1, myTexPos).rgb;
    vec3 diffuse = diffuseIntensity * diff * texture(texture_diffuse1, myTexPos).rgb;
    vec3 specular = specularIntensity * spec * texture(texture_specular1, myTexPos).rgb;
    FragColor = vec4(ambient+diffuse+specular,1.0f);
}