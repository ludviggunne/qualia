#version 450 core

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D imageOutput;

uniform vec4 uSeed;
uniform float uSection;
uniform int uIterations;
uniform float uRotation;
uniform float uZoom;
uniform vec2 uCenter;
uniform vec2 uClipping;
uniform ivec2 uOffset;
uniform ivec2 uSize;

vec4 qmul(vec4 a, vec4 b) {

    return vec4(
        a.x * b.x - a.y * b.y - a.z * b.z - a.w * b.w,
        a.x * b.y + a.y * b.x + a.z * b.w - a.w * b.z,
        a.x * b.z - a.y * b.w + a.z * b.x + a.w * b.y,
        a.x * b.w + a.y * b.z - a.z * b.y + a.w * b.x
    ); 
}

void main() {

    uint steps = 2 * uSize.x;
    ivec2 coord = uOffset + ivec2(gl_GlobalInvocationID.xy);
    if (coord.x >= uSize.x || coord.y >= uSize.y) return;

    vec2 p = uCenter + (vec2(coord) / vec2(uSize) - vec2(0.5, 0.5)) * uZoom;

    bool inc = false;
    int depth = 0;
    vec4 normal = vec4(0.0, 0.0, 0.0, 0.0);
    while (depth < steps) {

        float fdepth = uClipping.x + float(depth) / float(steps) * (uClipping.y - uClipping.x);
        vec4 _q = vec4(p.x, p.y, fdepth, uSection);
        vec4 q = vec4(cos(uRotation) * _q.x - sin(uRotation) * _q.z, _q.y, sin(uRotation) * _q.x + cos(uRotation) * _q.z, uSection);
        normal = q;

        inc = true;
        for (int i = 0; i < uIterations; i++) {

            q = qmul(q, q) + uSeed;

            if (length(q) > 4.0) {

                inc = false;
                break;
            }
        }

        if (inc == true) {
            break;
        }
        depth += 1;
    }

    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    if (inc) {

        float lum = 1.0  - float(depth) / float(steps);
        color = vec4(lum); //vec4(0.5 * (vec3(1.0, 1.0, 1.0) + normalize(normal).xyz), 1.0) * vec4(lum, lum, lum, 1.0); 
    }

    imageStore(imageOutput, coord, color);
}
