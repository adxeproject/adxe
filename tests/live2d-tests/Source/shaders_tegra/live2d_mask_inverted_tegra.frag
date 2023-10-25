#version 310 es
#extension GL_NV_shader_framebuffer_fetch : enable
precision mediump float;

layout(location = TEXCOORD0) in vec2 v_texCoord;
layout(location = TEXCOORD1) in vec4 v_clipPos;

layout(location = SV_Target0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_tex0;
layout(binding = 1) uniform sampler2D u_tex1;

layout(std140) uniform fs_ub {
    vec4 u_channelFlag;
    vec4 u_baseColor;
};

void main()
{
    vec4 col_formask = texture(u_tex0 , v_texCoord) * u_baseColor;
    col_formask.rgb = col_formask.rgb  * col_formask.a ;
    vec4 clipMask = (1.0 - texture(u_tex1, v_clipPos.xy / v_clipPos.w)) * u_channelFlag;
    float maskVal = clipMask.r + clipMask.g + clipMask.b + clipMask.a;
    col_formask = col_formask * (1.0 - maskVal);
    FragColor = col_formask;
}