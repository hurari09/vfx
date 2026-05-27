#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 SkyboxTexCoords;
in vec4 FragPosLightSpace;

in VS_OUT {
    vec3 TangentLightPos;
    vec3 TangentLight2Pos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

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

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform sampler2D normalMap;
uniform samplerCube skybox;

uniform Material material;
uniform Light light;
uniform Light light2;
uniform vec3 viewPos;

uniform bool isFloor;
uniform bool isTeapot;
uniform bool isNormalMapped;
uniform bool useReflection;
uniform bool isCartoon;
uniform bool useMultipleLights;
uniform bool isStar;

uniform bool isDepthPass;
uniform bool isSkybox;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0) return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    
    return shadow;
}

vec3 CalculateLightContribution(Light l, vec3 normal, vec3 lightDir, vec3 viewDir, vec3 color, float shadow, bool cartoon)
{
    vec3 ambient = l.ambient * material.diffuse;
    if (!isTeapot && !isNormalMapped) {
        ambient = 0.2 * color;
    }
    
    float diff = max(dot(normal, lightDir), 0.0);
    if (cartoon) {
        if (diff > 0.8) diff = 0.9;
        else if (diff > 0.6) diff = 0.7;
        else if (diff > 0.4) diff = 0.5;
        else if (diff > 0.2) diff = 0.3;
        else diff = 0.1;
    }
    
    vec3 diffuse = l.diffuse * (diff * material.diffuse);
    if (!isTeapot && !isNormalMapped) {
        diffuse = diff * color;
    }
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = l.specular * (spec * material.specular);
    if (!isTeapot && !isNormalMapped) {
        specular = vec3(0.4) * spec;
    }
    
    return ambient + (1.0 - shadow) * (diffuse + specular);
}

void main()
{           
    if (isDepthPass) {
        return;
    }

    if (isSkybox) {
        FragColor = texture(skybox, SkyboxTexCoords);
        return;
    }

    vec3 color = texture(diffuseTexture, TexCoords).rgb;
    
    if (isStar) {
        FragColor = vec4(color, 1.0);
        return;
    }

    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 mainLightDir = normalize(light.position - FragPos);
    vec3 subLightDir = normalize(light2.position - FragPos);

    if (isNormalMapped) {
        normal = texture(normalMap, TexCoords).rgb;
        normal = normalize(normal * 2.0 - 1.0);
        viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
        mainLightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
        subLightDir = normalize(fs_in.TangentLight2Pos - fs_in.TangentFragPos);
        color = texture(diffuseTexture, TexCoords).rgb;
    }
    
    float shadow = ShadowCalculation(FragPosLightSpace, normal, mainLightDir);       
    
    vec3 finalLighting = CalculateLightContribution(light, normal, mainLightDir, viewDir, color, shadow, isCartoon);
    
    if (useMultipleLights) {
        finalLighting += CalculateLightContribution(light2, normal, subLightDir, viewDir, color, 0.0, isCartoon);
    }
    
    if (useReflection || isFloor) {
        vec3 I = normalize(FragPos - viewPos);
        vec3 R = reflect(I, normalize(Normal));
        vec3 reflectionColor = texture(skybox, R).rgb;
        if (isFloor) {
            finalLighting = mix(finalLighting, reflectionColor, 0.15);
        } else {
            finalLighting = mix(finalLighting, reflectionColor, 0.5);
        }
    }

    FragColor = vec4(finalLighting, 1.0);
}