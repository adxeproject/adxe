#version 310 es
precision highp float;
precision highp int;
layout(location = TEXCOORD0) in vec2 v_texCoord;
layout(location = TEXCOORD1) in float v_fogFactor;


layout(binding = 0) uniform sampler2D u_tex0;

layout(std140) uniform fs_ub {
    vec4 u_fogColor;
};

layout(location = SV_Target0) out vec4 FragColor;

void main (void)
{
    vec4 finalColor = texture(u_tex0, v_texCoord);
    FragColor     = mix(u_fogColor, finalColor, v_fogFactor );        //out put finalColor
}
