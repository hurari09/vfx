#version 330 core

// 1. 정점 속성 레이아웃
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in ivec4 aBoneIds;   // 캐릭터 애니메이션용 본 ID
layout (location = 5) in vec4 aWeights;   // 캐릭터 애니메이션용 가중치

// 2. 출력 데이터 구조체 (프래그먼트 셰이더로 전달)
out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec4 FragPosLightSpace; // 섀도우 매핑용
    mat3 TBN;               // 노말 매핑용 탄젠트 공간 행렬
} vs_out;

// 3. Uniform 변수들
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // 섀도우 매핑용 광원 공간 변환 행렬

const int MAX_BONES = 100;
uniform mat4 boneMatrices[MAX_BONES]; // 애니메이션 본 행렬 배열
uniform bool isAnimated;              // 애니메이션 적용 여부 플래그

void main()
{
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);
    
    // [캐릭터 애니메이션] 본 가중치 연산
    if(isAnimated) {
        for(int i = 0 ; i < 4 ; i++) {
            if(aBoneIds[i] == -1) continue;
            if(aBoneIds[i] >= MAX_BONES) {
                totalPosition = vec4(aPos, 1.0);
                break;
            }
            vec4 localPosition = boneMatrices[aBoneIds[i]] * vec4(aPos, 1.0);
            totalPosition += localPosition * aWeights[i];
            
            vec3 localNormal = mat3(boneMatrices[aBoneIds[i]]) * aNormal;
            totalNormal += localNormal * aWeights[i];
        }
    } else {
        totalPosition = vec4(aPos, 1.0);
        totalNormal = aNormal;
    }

    // 기본 위치 계산
    vs_out.FragPos = vec3(model * totalPosition);
    vs_out.TexCoords = aTexCoords;
    
    // [섀도우 매핑] 광원 기준의 정점 위치 계산
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    
    // [노말 매핑] TBN 행렬 구성
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * totalNormal);
    T = normalize(T - dot(T, N) * N); // 그람-슈미트 직교화
    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);
    
    gl_Position = projection * view * model * totalPosition;
}