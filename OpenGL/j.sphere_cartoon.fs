#version 330 core
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 Normal;  
in vec3 FragPos;  
  
uniform Material material;
uniform Light light;
uniform vec3 eyePos;

void main()
{

    // diffuse term
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    if (diff > 0.8f) diff = 0.9f;
    else if (diff > 0.6f) diff = 0.7f;
    else if (diff > 0.4f) diff = 0.5f;
    else if (diff > 0.2f) diff = 0.3f;
    else diff = 0.1f;
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

	// specular term 
    vec3 View = normalize(eyePos - FragPos);
	vec3 refl = 2.0 * norm * dot(norm, lightDir) - lightDir; //    vec3 reflectDir = reflect(-lightDir, norm);  
 	float spec = pow(max(dot(refl, View), 0.0), material.shininess); 
 /*
    if (spec > 0.8f) spec = 0.9f;
    else if (spec > 0.6f) spec = 0.7f;
    else if (spec > 0.4f) spec = 0.5f;
    else if (spec > 0.2f) spec = 0.3f;
    else spec = 0.1f;
    */
	vec3 specular = light.specular * (spec * material.specular);

	// ambient term
     vec3 ambient = light.ambient * material.ambient;
            
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
} 
