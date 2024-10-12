// GLSL fragment shader
#version 400 core

// Input constant data
layout(std140) uniform Scene
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
in vec3 teViewPos;
in vec3 teNormal;
in vec3 teTangent;
in vec3 teBitangent;
in vec2 teTexCoord;

out vec4 fragColor;

// Input textures
uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;

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
    vec4 albedo = texture(colorMap, teTexCoord);
    vec3 normal = normalize(texture(normalMap, teTexCoord).rgb * 2.0 - 1.0);
    float roughness = texture(specularMap, teTexCoord).r;
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
