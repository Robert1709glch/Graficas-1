#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    sampler2D emission;
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform int lightType;
uniform float amount;

void main()
{
    vec4 texColor = texture(material.diffuse, TexCoords);
    // ambient
    vec3 ambient = light.ambient * texColor.rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
  
    // emission
    vec3 emission = texture(material.emission, TexCoords).rgb;
        


    //SpotLight
    float intensity = 1.0;
    vec3 specular = vec3(1.0f);
    vec3 result = vec3(1.0f);

    vec3 n = normalize(Normal);
    vec3 l = normalize(lightDir);
    vec3 e = normalize(viewPos);
    // inside the cone?
   
    if(lightType == 1) //Spot
    {
        intensity = max(dot(n,l), 0.0);
        if (intensity > 0.0) {
            vec3 h = normalize(l + e);
            float intSpec = max(dot(h,n), 0.0);
            specular = specular * pow(intSpec, material.shininess);
        }
        result = max(intensity * diffuse + specular, ambient);

    }
    else if(lightType == 2)
    {
 
        intensity = max(dot(n,lightDir), 0.0);
 
        // if the vertex is lit compute the specular color
        if (intensity > 0.0) {
            // compute the half vector
            vec3 h = normalize(lightDir + e);  
            // compute the specular term into spec
            float intSpec = max(dot(h,n), 0.0);
            specular = light.specular * pow(intSpec,material.shininess);
        }
        result = max(intensity * diffuse + specular, ambient);

    }
    else if(lightType == 3) //Caster
    {
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  

        // spotlight (soft edges)
        float theta = dot(lightDir, normalize(-light.direction)); 
        float epsilon = (light.cutOff - light.outerCutOff);
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;
    
        // attenuation
        float distance    = length(light.position - FragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
        ambient  *= attenuation; 
        diffuse   *= attenuation;
        specular *= attenuation;
        result = diffuse + specular + ambient;

    }
    else
    {
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        specular = light.specular * spec * texture(material.specular, TexCoords).rgb;  
        result = diffuse + specular + emission, ambient;

    }

    if(texColor.a < 0.1)
       discard;

    FragColor = vec4(result, 1.0);
} 