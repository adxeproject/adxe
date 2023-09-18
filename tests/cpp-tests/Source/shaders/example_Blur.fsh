#version 310 es
precision highp float;
precision highp int;

layout(location = COLOR0) in vec4 v_color;
layout(location = TEXCOORD0) in vec2 v_texCoord;


layout(binding = 0) uniform sampler2D u_tex0;

layout(std140) uniform fs_ub {
    vec2 resolution;
    float blurRadius;
    float sampleNum;
};

vec4 blur(vec2);

layout(location = SV_Target0) out vec4 FragColor;

void main(void)
{
    vec4 col = blur(v_texCoord); //* v_color.rgb;
    FragColor = vec4(col) * v_color;
}

vec4 blur(vec2 p)
{
    if (blurRadius > 0.0 && sampleNum > 1.0)
    {
        vec4 col = vec4(0);
        vec2 unit = 1.0 / resolution.xy;
        
        float r = blurRadius;
        float sampleStep = r / sampleNum;
        
        float count = 0.0;
        
        for(float x = -r; x < r; x += sampleStep)
        {
            for(float y = -r; y < r; y += sampleStep)
            {
                float weight = (r - abs(x)) * (r - abs(y));
                col += texture(u_tex0, p + vec2(x * unit.x, y * unit.y)) * weight;
                count += weight;
            }
        }
        
        return col / count;
    }
    
    return texture(u_tex0, p);
}
