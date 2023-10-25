#version 310 es

layout(location = POSITION) in vec4 a_position;
layout(location = TEXCOORD0) in vec2 a_texCoord;

layout(location = TEXCOORD0) out vec2 v_texCoord;
layout(location = TEXCOORD1) out vec4 v_myPos;

layout(std140) uniform vs_ub {
    mat4 u_clipMatrix;
};

void main()
{
    vec4 pos = vec4(a_position.x, a_position.y, 0.0, 1.0);
    gl_Position = u_clipMatrix * pos;
    v_myPos = u_clipMatrix * pos;
    v_texCoord = a_texCoord;
    v_texCoord.y = 1.0 - v_texCoord.y;
}