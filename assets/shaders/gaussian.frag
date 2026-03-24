#version 430 core

in vec3 vColor;
in float vOpacity;
in vec3 vInvCov2D;
in float vHalfExtentPx;
in vec2 vLocalPx;
uniform int u_useAnisotropic;

layout(location = 0) out vec4 outColor;

void main() {
    float alpha = 0.0;
    float alphaThreshold = 1.0 / 255.0;
    if (u_useAnisotropic != 0) {
        mat2 invCov = mat2(
            vInvCov2D.x, vInvCov2D.y,
            vInvCov2D.y, vInvCov2D.z
        );
        float q = dot(vLocalPx, invCov * vLocalPx);
        alpha = exp(-0.5 * min(max(q, 0.0), 64.0)) * vOpacity;
    } else {
        vec2 uv = vLocalPx / max(vHalfExtentPx, 1e-5);
        float r2 = dot(uv, uv);
        if (r2 > 1.0) {
            discard;
        }

        float falloff = 3.2;
        alpha = exp(-r2 * falloff) * vOpacity;
    }

    if (alpha < alphaThreshold) {
        discard;
    }
    vec3 color = max(vColor, vec3(0.0));
    outColor = vec4(color * alpha, alpha);
}
