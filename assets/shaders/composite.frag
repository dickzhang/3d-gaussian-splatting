#version 430 core

uniform sampler2D u_accumTex;

layout(location = 0) out vec4 outColor;

void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy);
    vec4 accum = texelFetch(u_accumTex, pixel, 0);
    float alpha = max(accum.a, 1e-6);
    vec3 rgb = accum.rgb / alpha;
    outColor = vec4(rgb, accum.a);
}
