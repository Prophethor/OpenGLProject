#version 330 core

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 fragPos;

out vec4 FragColor;

uniform float shininess;
uniform vec3 viewPos;
uniform PointLight pointLight;
uniform DirLight dirLight;
uniform vec3 lightColor;
uniform vec3 myColor;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    vec3 normal = vec3(0.0f,1.0f,0.0f);
    vec3 viewDir = normalize(viewPos - fragPos);
    
    vec3 result = CalcDirLight(dirLight, normal, viewDir);
    result += CalcPointLight(pointLight, normal, fragPos, viewDir);   
    
    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = normalize(reflect(-lightDir, normal));
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = light.ambient * myColor;
    vec3 diffuse = light.diffuse * diff * myColor;
    vec3 specular = light.specular * spec * myColor;
    return (ambient + diffuse + specular)*lightColor;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * myColor;
    vec3 diffuse = light.diffuse * diff * myColor;
    vec3 specular = light.specular * spec * myColor;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse*lightColor + specular*lightColor);
}