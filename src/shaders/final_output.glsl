#version 410 core

out vec4 colour;

uniform vec2 u_Resolution;
uniform sampler2D u_PT_Texture;

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_Resolution);
    vec3 final_colour = texture(u_PT_Texture, uv).rgb;
    colour = vec4(final_colour, 1.0);
}