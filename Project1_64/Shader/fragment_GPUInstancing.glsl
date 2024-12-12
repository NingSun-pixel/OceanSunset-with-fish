#version 330 core

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 VertColor;
out vec4 FragColor;

// Uniforms
uniform vec3 lightDirection;  // 平行光的方向
uniform vec3 lightColor;      // 平行光的颜色
uniform vec3 viewPos;         // 观察者位置
uniform sampler2D albedoMap;  // 漫反射贴图
uniform sampler2D normalMap;  // 法线贴图

// 材质属性
uniform float metallic;
uniform float roughness;
uniform float ambientOcclusion;

// Helper functions
vec3 calculateNormal()
{
    // 如果有法线贴图，则使用它，否则使用顶点法线
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

// Fresnel-Schlick approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Distribution function
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

// Geometry functions
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
    // 获取材质属性
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    vec3 N = Normal;// calculateNormal();
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(-lightDirection); // 平行光方向需要反转
    vec3 H = normalize(V + L);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   // Normal Distribution Function
    float G = GeometrySmith(N, V, L, roughness);    // Geometry function
    vec3 F0 = vec3(0.04);                           // Fresnel reflectance at normal incidence
    F0 = mix(F0, albedo, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0); // Fresnel-Schlick approximation

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    // kS is the Fresnel-Schlick approximation
    vec3 kS = F;
    // kD is the diffuse component, calculated as 1 - kS
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // Lambertian diffuse
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * albedo / 3.14159265359;

    // 计算最终颜色
    vec3 ambient = vec3(0.03) * albedo * ambientOcclusion;
    vec3 color = ambient + (diffuse + specular) * lightColor * NdotL + albedo * 0.05;

    // 伽马校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
