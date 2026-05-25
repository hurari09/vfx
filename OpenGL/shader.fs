#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    mat3 TBN;
} fs_in;

// 텍스처 및 맵
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;    // 노말 맵
uniform sampler2D shadowMap;          // 섀도우 맵
uniform samplerCube skybox;           // 큐브 맵 (환경 매핑용)

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool useNormalMap;
uniform bool useReflection;           // 큐브 매핑 반사 적용 여부

// [섀도우 매핑] 그림자 판정 함수
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 투영 좌표계로 변환 (-1 ~ 1 범위)
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 0 ~ 1 범위로 변환
    projCoords = projCoords * 0.5 + 0.5;
    
    // 섀도우 맵 영역 밖은 그림자 제외
    if(projCoords.z > 1.0) return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    
    // 섀도우 아크네 방지를 위한 바이어스 적용
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    
    return shadow;
}

void main()
{           
    // 1. [노말 매핑] 법선 벡터 추출 및 변환
    vec3 normal = fs_in.TBN[2]; // 기본 Normal
    if(useNormalMap) {
        normal = texture(texture_normal1, fs_in.TexCoords).rgb;
        normal = normal * 2.0 - 1.0;   // 0~1 단위를 -1~1로 변환
        normal = normalize(fs_in.TBN * normal); // 탄젠트 공간 -> 월드 공간
    }
    
    // 기본 색상 추출
    vec3 color = texture(texture_diffuse1, fs_in.TexCoords).rgb;
    
    // Ambient (환경광)
    vec3 ambient = 0.15 * color;
    
    // Diffuse (난반사광)
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    
    // Specular (경면광)
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec;
    
    // 2. [섀도우 매핑] 그림자 계산
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);    
    
    // 3. [큐브 매핑] 스카이박스 환경 반사 계산
    if(useReflection) {
        vec3 R = reflect(-viewDir, normal);
        vec3 reflectionColor = texture(skybox, R).rgb;
        lighting = mix(lighting, reflectionColor, 0.2); // 기존 빛 연산과 반사광 합성
    }
    
    FragColor = vec4(lighting, 1.0);
}