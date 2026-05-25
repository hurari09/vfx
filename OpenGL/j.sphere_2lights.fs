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
uniform Light light2;
uniform vec3 eyePos;

void main()
{
    // **** light 1 ****
    // diffuse term 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * material.diffuse);

	// specular term 
    vec3 View = normalize(eyePos - FragPos);
	vec3 refl = 2.0 * norm * dot(norm, lightDir) - lightDir; //    vec3 reflectDir = reflect(-lightDir, norm);  
 	float spec = pow(max(dot(refl, View), 0.0), material.shininess); 
	vec3 specular = light.specular * (spec * material.specular);

	// ambient term
    vec3 ambient = light.ambient * material.ambient;
            
    vec3 result1 = ambient + diffuse + specular;

    // **** light 2 ****
    // diffuse term 
    norm = normalize(Normal);
    lightDir = normalize(light2.position - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    diffuse = light2.diffuse * (diff * material.diffuse);

	// specular term 
    View = normalize(eyePos - FragPos);
	refl = 2.0 * norm * dot(norm, lightDir) - lightDir; //    vec3 reflectDir = reflect(-lightDir, norm);  
 	spec = pow(max(dot(refl, View), 0.0), material.shininess); 
	specular = light2.specular * (spec * material.specular);

	// ambient term
    ambient = light2.ambient * material.ambient;
        
    vec3 result2 = ambient + diffuse + specular;

    // add two results
    FragColor = vec4(result1+result2, 1.0);
    //FragColor = vec4(result1, 1.0);
} 
