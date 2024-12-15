#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

// Uniforms for directional light
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float smoothness;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D RoughnessMap;

// Fog parameters
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogHeightStart;
uniform float fogHeightEnd;
uniform float fogDistanceStart;
uniform float fogDistanceEnd;

// Material properties
uniform float metallic;
uniform float roughness;
uniform float ambientOcclusion;

// Point light parameters
#define MAX_POINT_LIGHTS 8
uniform vec3 pointLightPositions[MAX_POINT_LIGHTS];
uniform vec3 pointLightColors[MAX_POINT_LIGHTS];
uniform float pointLightIntensities[MAX_POINT_LIGHTS];
uniform float pointLightRadii[MAX_POINT_LIGHTS];
uniform int numPointLights;

// Helper functions
vec3 calculateNormal()
{
    vec3 norm = Normal;
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    vec3 N = calculateNormal();
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(V + L);
    float Roughness_Tex = texture(RoughnessMap, TexCoords).r;

    float NDF = DistributionGGX(N, H, Roughness_Tex);
    float G = GeometrySmith(N, V, L, Roughness_Tex);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * albedo / 3.14159265359;

    vec3 ambient = vec3(0.03) * albedo * ambientOcclusion;
    vec3 color = ambient + (diffuse + specular) * lightColor * NdotL * smoothness + albedo * 0.05;

    // 遍历所有点光源
    for (int i = 0; i < numPointLights; i++) {
        vec3 pointLightPos = pointLightPositions[i];
        vec3 pointLightColor = pointLightColors[i];
        float pointLightIntensity = pointLightIntensities[i];
        float pointLightRadius = pointLightRadii[i];

        vec3 Lp = normalize(pointLightPos - FragPos);
        float distance = length(pointLightPos - FragPos);
        float attenuation = 1.0 / (1.0 + (distance / pointLightRadius) * (distance / pointLightRadius));
        float NdotLp = max(dot(N, Lp), 0.0);

        vec3 pointDiffuse = kD * albedo / 3.14159265359;
        vec3 pointSpecular = (NDF * GeometrySmith(N, V, Lp, Roughness_Tex) * fresnelSchlick(max(dot(H, V), 0.0), F0)) / denominator;

        vec3 pointLight = (pointDiffuse + pointSpecular) * pointLightColor * attenuation * pointLightIntensity * NdotLp;

        color += pointLight;
    }

    // 伽马校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
