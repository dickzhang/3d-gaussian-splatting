#version 430 core

struct GPUViewSplat {
    vec4 clipPos;
    vec4 colorOpacity;
    vec4 invCovHalf;
    vec4 basisPacked;
};

layout(std430, binding = 2) readonly buffer ViewDataBuffer {
    GPUViewSplat viewSplats[];
};

uniform vec2 u_viewportSize;
uniform int u_useAnisotropic;

out vec3 vColor;
out float vOpacity;
out vec3 vInvCov2D;
out float vHalfExtentPx;
out vec2 vLocalPx;

void main() {
    GPUViewSplat viewData = viewSplats[gl_InstanceID];
    if (viewData.colorOpacity.a <= 0.0) {
        gl_Position = vec4(3.0, 3.0, 3.0, 1.0);
        vColor = vec3(0.0);
        vOpacity = 0.0;
        vInvCov2D = vec3(1.0, 0.0, 1.0);
        vHalfExtentPx = 1.0;
        vLocalPx = vec2(0.0);
        return;
    }

    vColor = viewData.colorOpacity.rgb;
    vOpacity = viewData.colorOpacity.a;
    vInvCov2D = viewData.invCovHalf.xyz;
    vHalfExtentPx = viewData.invCovHalf.w;

    vec2 basisMajor = viewData.basisPacked.xy;
    vec2 basisMinor = viewData.basisPacked.zw;

    const vec2 quad[6] = vec2[6](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0, -1.0),
        vec2( 1.0,  1.0)
    );
    vec2 quadUv = quad[gl_VertexID];
    vec2 local = basisMajor * quadUv.x + basisMinor * quadUv.y;
    vLocalPx = local;

    vec2 ndcOffset = (local / u_viewportSize) * 2.0;
    gl_Position = vec4(viewData.clipPos.xy + ndcOffset * viewData.clipPos.w, viewData.clipPos.zw);
}
