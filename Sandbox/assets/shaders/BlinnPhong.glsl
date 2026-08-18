// BlinnPhong.glsl —— 匹配 Mesh::Draw 已设置的 uniform 契约（单个方向光 + 贴图/纯色材质二选一）
#type vertex
#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat3 u_NormalMatrix;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoords;
out mat3 v_TBN;

void main() {
    vec4 worldPos = u_Transform * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    v_TexCoords = a_TexCoords;

    vec3 N = normalize(u_NormalMatrix * a_Normal);
    v_Normal = N;

    // TBN 供法线贴图用；Gram-Schmidt 让 T 与 N 保持垂直（非均匀缩放后二者会偏斜）
    vec3 T = normalize(u_NormalMatrix * a_Tangent);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    v_TBN = mat3(T, B, N);

    gl_Position = u_ViewProjection * worldPos;
}

#type fragment
#version 450 core
in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoords;
in mat3 v_TBN;

out vec4 o_Color;

uniform sampler2D u_Material_DiffuseMap;
uniform int u_Material_HasDiffuseMap;
uniform sampler2D u_Material_SpecularMap;
uniform int u_Material_HasSpecularMap;
uniform sampler2D u_Material_NormalMap;
uniform int u_Material_HasNormalMap;

uniform vec3 u_Material_Ambient;
uniform vec3 u_Material_Diffuse;
uniform vec3 u_Material_Specular;
uniform float u_Material_Shininess;

uniform vec3 u_DirLight_Direction;
uniform vec3 u_DirLight_Color;
uniform float u_DirLight_Intensity;

uniform vec3 u_ViewPos;

void main() {
    // 法线：有法线贴图则采样切线空间并转到世界空间，否则用插值法线
    vec3 N;
    if (u_Material_HasNormalMap == 1) {
        vec3 tangentNormal = texture(u_Material_NormalMap, v_TexCoords).rgb * 2.0 - 1.0;
        N = normalize(v_TBN * tangentNormal);
    }
    else {
        N = normalize(v_Normal);
    }

    // 材质反射率：有贴图则采样，否则用纯色参数（二者互斥，由 Mesh::Draw 的 Has* 开关决定）
    vec3 diffuseColor = u_Material_HasDiffuseMap == 1
                             ? texture(u_Material_DiffuseMap, v_TexCoords).rgb
                             : u_Material_Diffuse;
    vec3 specularColor = u_Material_HasSpecularMap == 1
                              ? texture(u_Material_SpecularMap, v_TexCoords).rgb
                              : u_Material_Specular;

    vec3 L = normalize(-u_DirLight_Direction);
    vec3 V = normalize(u_ViewPos - v_WorldPos);
    vec3 H = normalize(L + V);  // Blinn-Phong：半程向量代替反射向量，高光更平滑且无需求反射

    float diff = max(dot(N, L), 0.0);
    float spec = diff > 0.0 ? pow(max(dot(N, H), 0.0), u_Material_Shininess) : 0.0;

    vec3 ambient = u_Material_Ambient * diffuseColor;
    vec3 diffuse = diff * diffuseColor;
    vec3 specular = spec * specularColor;

    vec3 result = (ambient + diffuse + specular) * u_DirLight_Color * u_DirLight_Intensity;
    o_Color = vec4(result, 1.0);
}
