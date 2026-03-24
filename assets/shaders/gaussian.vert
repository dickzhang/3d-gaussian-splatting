#version 430 core

struct GPUSplat {
    vec4 posOpacity;
    vec4 scalePad;
    vec4 rotation;
    vec4 colorRadius;
    uint shPacked[24];
};

layout(std430, binding = 0) readonly buffer SplatBuffer {
    GPUSplat splats[];
};

layout(std430, binding = 1) readonly buffer SortedIndexBuffer {
    uint sortedIndices[];
};

uniform mat4 u_view;
uniform mat4 u_proj;
uniform vec2 u_viewportSize;
uniform float u_maxPointSize;
uniform int u_useAnisotropic;
uniform vec3 u_cameraPos;
uniform int u_shDegree;

out vec3 vColor;
out float vOpacity;
out vec3 vInvCov2D;
out float vHalfExtentPx;
out vec2 vLocalPx;

mat3 quatToMat3(vec4 q) {
    float x = q.x;
    float y = q.y;
    float z = q.z;
    float w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    return mat3(
        1.0 - 2.0 * (yy + zz), 2.0 * (xy + wz),       2.0 * (xz - wy),
        2.0 * (xy - wz),       1.0 - 2.0 * (xx + zz), 2.0 * (yz + wx),
        2.0 * (xz + wy),       2.0 * (yz - wx),       1.0 - 2.0 * (xx + yy)
    );
}

void main() {
    uint i = sortedIndices[gl_InstanceID];
    if (i == 0xFFFFFFFFu) {
        gl_Position = vec4(3.0, 3.0, 3.0, 1.0);
        vColor = vec3(0.0);
        vOpacity = 0.0;
        vInvCov2D = vec3(1.0, 0.0, 1.0);
        vHalfExtentPx = 1.0;
        vLocalPx = vec2(0.0);
        return;
    }

    GPUSplat splat = splats[i];
    vec4 po = splat.posOpacity;
    vec4 cr = splat.colorRadius;

    vec4 viewPos = u_view * vec4(po.xyz, 1.0);
    vec4 clipPos = u_proj * viewPos;

    float depth = max(0.01, -viewPos.z);

    float aspect = u_proj[0][0] / u_proj[1][1];
    float tanFovX = 1.0 / u_proj[0][0];
    float tanFovY = 1.0 / (u_proj[1][1] * aspect);
    float limX = 1.3 * tanFovX;
    float limY = 1.3 * tanFovY;
    float zAbs = max(0.01, abs(viewPos.z));
    float nx = clamp(viewPos.x / zAbs, -limX, limX);
    float ny = clamp(viewPos.y / zAbs, -limY, limY);
    vec3 clampedViewPos = vec3(nx * zAbs, ny * zAbs, viewPos.z);

    vec3 scales = max(vec3(0.00005), splat.scalePad.xyz);
    vec4 q = splat.rotation;
    float qLen = length(q);
    if (qLen < 1e-6) {
        q = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        q /= qLen;
    }
    mat3 rot = quatToMat3(q);
    mat3 covLocal = mat3(
        scales.x * scales.x, 0.0, 0.0,
        0.0, scales.y * scales.y, 0.0,
        0.0, 0.0, scales.z * scales.z
    );
    mat3 covWorld = rot * covLocal * transpose(rot);
    mat3 viewRot = mat3(u_view);
    mat3 covView = viewRot * covWorld * transpose(viewRot);

    float c00;
    float c01;
    float c11;
    if (u_useAnisotropic != 0) {
        float fx = 0.5 * u_viewportSize.x * u_proj[0][0];
        float fy = 0.5 * u_viewportSize.y * u_proj[1][1];
        float invZ = 1.0 / depth;
        float invZ2 = invZ * invZ;

        vec3 j0 = vec3(fx * invZ, 0.0, -fx * clampedViewPos.x * invZ2);
        vec3 j1 = vec3(0.0, fy * invZ, -fy * clampedViewPos.y * invZ2);

        vec3 covJ0 = covView * j0;
        vec3 covJ1 = covView * j1;
        c00 = max(1e-9, dot(j0, covJ0));
        c01 = dot(j0, covJ1);
        c11 = max(1e-9, dot(j1, covJ1));
        // Unity plugin low-pass term.
        c00 += 0.3;
        c11 += 0.3;
    } else {
        float k = (0.5 * u_viewportSize.y) / depth;
        float radius = max(0.0001, splat.colorRadius.a);
        float sigma = max(1.0, radius * k);
        float v = sigma * sigma;
        c00 = v;
        c01 = 0.0;
        c11 = v;
    }

    float trace = c00 + c11;
    float det = max(1e-16, c00 * c11 - c01 * c01);
    float disc = sqrt(max(0.0, 0.25 * trace * trace - det));
    float lambdaMax = max(1e-9, 0.5 * trace + disc);
    float lambdaMin = max(1e-9, 0.5 * trace - disc);

    const float maxFootprintPx = max(u_maxPointSize, 8192.0);
    float sigmaScale = 3.0;

    vec2 majorAxis;
    vec2 eigenVecRaw = vec2(c01, lambdaMax - c00);
    if (dot(eigenVecRaw, eigenVecRaw) > 1e-12) {
        majorAxis = normalize(eigenVecRaw);
    } else {
        majorAxis = (c00 >= c11) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    }
    vec2 minorAxis = vec2(-majorAxis.y, majorAxis.x);

    float majorRadiusPx = clamp(max(1.0, sigmaScale * sqrt(lambdaMax)), 1.0, maxFootprintPx);
    float minorRadiusPx = clamp(max(1.0, sigmaScale * sqrt(lambdaMin)), 1.0, maxFootprintPx);

    vec2 basisMajor = majorAxis * majorRadiusPx;
    vec2 basisMinor = minorAxis * minorRadiusPx;
    vHalfExtentPx = max(majorRadiusPx, minorRadiusPx);

    float det2 = det;
    if (det2 <= 1e-6) {
        float safeV = max(c00, c11);
        vInvCov2D = vec3(1.0 / safeV, 0.0, 1.0 / safeV);
    } else {
        float invDet = 1.0 / det2;
        vInvCov2D = vec3(
            clamp(c11 * invDet, -1e6, 1e6),
            clamp(-c01 * invDet, -1e6, 1e6),
            clamp(c00 * invDet, -1e6, 1e6)
        );
    }

    vec3 color = cr.rgb;
    if (u_shDegree > 0) {
        vec2 p0 = unpackHalf2x16(splat.shPacked[0]);
        vec2 p1 = unpackHalf2x16(splat.shPacked[1]);
        vec2 p2 = unpackHalf2x16(splat.shPacked[2]);
        vec2 p3 = unpackHalf2x16(splat.shPacked[3]);
        vec2 p4 = unpackHalf2x16(splat.shPacked[4]);
        vec2 p5 = unpackHalf2x16(splat.shPacked[5]);
        vec2 p6 = unpackHalf2x16(splat.shPacked[6]);
        vec2 p7 = unpackHalf2x16(splat.shPacked[7]);
        vec2 p8 = unpackHalf2x16(splat.shPacked[8]);
        vec2 p9 = unpackHalf2x16(splat.shPacked[9]);
        vec2 p10 = unpackHalf2x16(splat.shPacked[10]);
        vec2 p11 = unpackHalf2x16(splat.shPacked[11]);
        vec2 p12 = unpackHalf2x16(splat.shPacked[12]);
        vec2 p13 = unpackHalf2x16(splat.shPacked[13]);
        vec2 p14 = unpackHalf2x16(splat.shPacked[14]);
        vec2 p15 = unpackHalf2x16(splat.shPacked[15]);
        vec2 p16 = unpackHalf2x16(splat.shPacked[16]);
        vec2 p17 = unpackHalf2x16(splat.shPacked[17]);
        vec2 p18 = unpackHalf2x16(splat.shPacked[18]);
        vec2 p19 = unpackHalf2x16(splat.shPacked[19]);
        vec2 p20 = unpackHalf2x16(splat.shPacked[20]);
        vec2 p21 = unpackHalf2x16(splat.shPacked[21]);
        vec2 p22 = unpackHalf2x16(splat.shPacked[22]);
        vec2 p23 = unpackHalf2x16(splat.shPacked[23]);

        vec3 sh1_0 = vec3(p0.x, p0.y, p1.x);
        vec3 sh1_1 = vec3(p1.y, p2.x, p2.y);
        vec3 sh1_2 = vec3(p3.x, p3.y, p4.x);
        vec3 sh2_0 = vec3(p4.y, p5.x, p5.y);
        vec3 sh2_1 = vec3(p6.x, p6.y, p7.x);
        vec3 sh2_2 = vec3(p7.y, p8.x, p8.y);
        vec3 sh2_3 = vec3(p9.x, p9.y, p10.x);
        vec3 sh2_4 = vec3(p10.y, p11.x, p11.y);
        vec3 sh3_0 = vec3(p12.x, p12.y, p13.x);
        vec3 sh3_1 = vec3(p13.y, p14.x, p14.y);
        vec3 sh3_2 = vec3(p15.x, p15.y, p16.x);
        vec3 sh3_3 = vec3(p16.y, p17.x, p17.y);
        vec3 sh3_4 = vec3(p18.x, p18.y, p19.x);
        vec3 sh3_5 = vec3(p19.y, p20.x, p20.y);
        vec3 sh3_6 = vec3(p21.x, p21.y, p22.x);
        vec3 sh3_7 = vec3(p22.y, p23.x, p23.y);

        vec3 viewDir = po.xyz - u_cameraPos;
        float dirLen = length(viewDir);
        if (dirLen > 1e-6) {
            viewDir /= dirLen;
            const float c1 = 0.4886025119;
            color += sh1_0 * (-c1 * viewDir.y);
            color += sh1_1 * (c1 * viewDir.z);
            color += sh1_2 * (-c1 * viewDir.x);

            if (u_shDegree >= 2) {
                float x = viewDir.x;
                float y = viewDir.y;
                float z = viewDir.z;
                float xx = x * x;
                float yy = y * y;
                float zz = z * z;

                const float c2_0 = 1.0925484306;
                const float c2_1 = -1.0925484306;
                const float c2_2 = 0.3153915653;
                const float c2_3 = -1.0925484306;
                const float c2_4 = 0.5462742153;

                color += sh2_0 * (c2_0 * x * y);
                color += sh2_1 * (c2_1 * y * z);
                color += sh2_2 * (c2_2 * (2.0 * zz - xx - yy));
                color += sh2_3 * (c2_3 * x * z);
                color += sh2_4 * (c2_4 * (xx - yy));

                if (u_shDegree >= 3) {
                    const float c3_0 = -0.5900435899;
                    const float c3_1 = 2.8906114426;
                    const float c3_2 = -0.4570457995;
                    const float c3_3 = 0.3731763326;
                    const float c3_4 = -0.4570457995;
                    const float c3_5 = 1.4453057213;
                    const float c3_6 = -0.5900435899;

                    color += sh3_0 * (c3_0 * y * (3.0 * xx - yy));
                    color += sh3_1 * (c3_1 * x * y * z);
                    color += sh3_2 * (c3_2 * y * (4.0 * zz - xx - yy));
                    color += sh3_3 * (c3_3 * z * (2.0 * zz - 3.0 * xx - 3.0 * yy));
                    color += sh3_4 * (c3_4 * x * (4.0 * zz - xx - yy));
                    color += sh3_5 * (c3_5 * z * (xx - yy));
                    color += sh3_6 * (c3_6 * x * (xx - 3.0 * yy));
                }
            }
        }
    }

    vColor = max(color, vec3(0.0));
    vOpacity = po.w;

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
    gl_Position = vec4(clipPos.xy + ndcOffset * clipPos.w, clipPos.zw);
}
