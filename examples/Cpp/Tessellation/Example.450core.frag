// GLSL fragment shader
#version 450 core

// Input constant data
layout(std140, binding = 1) uniform Scene
{
    mat4    vpMatrix;
    mat4    vMatrix;
    mat4    wMatrix;
    vec3    lightVec;
    float   texScale;
    float   tessLevelInner;
    float   tessLevelOuter;
    float   maxHeightFactor;
    float   shininessPower;
};

// Input and output attributes
layout(location = 0) in vec3 teViewPos;
layout(location = 1) in vec3 teNormal;
layout(location = 2) in vec3 teTangent;
layout(location = 3) in vec3 teBitangent;
layout(location = 4) in vec2 teTexCoord;

layout(location = 0) out vec4 fragColor;

// Input textures
layout(binding = 2) uniform sampler linearSampler;
layout(binding = 3) uniform texture2D colorMap;
layout(binding = 4) uniform texture2D normalMap;
layout(binding = 5) uniform texture2D specularMap;

vec4 PhongShading(vec4 albedo, vec3 normal, float shininess, vec3 viewVec)
{
    // Diffuse lighting
    float lambertian    = max(0.0, dot(normal, lightVec));
    vec3 diffuse        = albedo.rgb * lambertian;

    // Specular lighting
    vec3 reflectVec   = normalize(reflect(lightVec, normal));
    vec3 halfVec      = normalize(lightVec - viewVec);
    float specAngle   = max(0.0, dot(normal, halfVec));
    vec3 specular     = vec3(shininess * pow(specAngle, shininessPower));

    return vec4(diffuse + specular, albedo.a);
}

void main()
{
    // Sample color, normal, and specular maps
    vec4 albedo = texture(sampler2D(colorMap, linearSampler), teTexCoord);
    vec3 normal = normalize(texture(sampler2D(normalMap, linearSampler), teTexCoord).rgb * 2.0 - 1.0);
    float roughness = texture(sampler2D(specularMap, linearSampler), teTexCoord).r;
    float shininess = 1.0 - roughness;

    // Transform normal from world-space into tangent-space
    mat3 tangentSpace = mat3(
        normalize(teTangent),
        normalize(teBitangent),
        normalize(teNormal)
    );
    normal = normal * tangentSpace;

    // Compute simple phong shading
    fragColor = PhongShading(albedo, normal, shininess, normalize(teViewPos));
}
