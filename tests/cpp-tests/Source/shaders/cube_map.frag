#version 310 es
precision highp float;
precision highp int;

layout(location = 0) in vec3 v_reflect;
layout(binding = 0) uniform samplerCube u_cubeTex;

layout(std140, binding = 0) uniform fs_ub {
    vec4 u_color;
};

layout(location = 0) out vec4 FragColor;

void main(void)
{
    FragColor = texture(u_cubeTex, v_reflect) * u_color;
}