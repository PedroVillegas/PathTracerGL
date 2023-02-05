#version 410 core

out vec4 colour;

uniform vec2 u_Resolution;
uniform sampler2D u_PT_Texture;

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_Resolution);
    vec4 final_colour = texture(u_PT_Texture, uv);
    colour = vec4(final_colour.rgb, 1.0);
}