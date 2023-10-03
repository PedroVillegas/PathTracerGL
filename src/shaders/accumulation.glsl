#version 460 core

out vec4 colour;

uniform vec2 u_Resolution;
uniform sampler2D u_PathTraceTexture;

void main()
{
    vec2 uv = (gl_FragCoord.xy / u_Resolution);
    vec3 prev_colour = texture(u_PathTraceTexture, uv).rgb;
    colour = vec4(prev_colour, 1.0);
}