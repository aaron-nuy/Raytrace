#version 430 core
#define PI 3.14159265359

in vec2 vPosition;

struct Material {
    vec3 albedo;
    float roughness;
    float metalic;
    float emissive;
    float specular;
};

struct Sphere {
    vec3 position;
    float radius;
    Material material;
};

struct Box {
    vec3 position;
    vec3 dimensions;
    Material material;
};

struct Triangle {
    vec3 p1;
    vec3 p2;
    vec3 p3;
    Material material;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Payload {
    float distance;
    vec3 normal;
    Material material;
};

layout(std140, binding = 0) uniform SceneBlock {
    mat4 viewInverse;
    vec4 cameraPos;
    float time;
    float aspectRatio;
    float frameCounter;
    int bounces;
    uint sphereNum;
    uint boxNum;
    uint triangleNum;
    uint state;
    bool reset;
} scene;

layout(std430, binding = 1) buffer SphereBlock {
    Sphere sphereList[];
};

layout(std430, binding = 2) buffer BoxBlock {
    Box boxList[];
};

layout(std430, binding = 3) buffer TriangleBlock {
    Triangle triangleList[];
};

uniform samplerCube cubemap;
uniform sampler2D text;

const float inf = 3.402823466e38;

// Fast PCG Random Number Generator
uint rngState;

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

float rand01() {
    rngState = hash(rngState);
    return float(rngState) * (1.0 / 4294967295.0);
}

Payload minPayload(Payload p1, Payload p2) {
    if (p1.distance < 0.0) return p2;
    if (p2.distance < 0.0) return p1;
    if (p1.distance < p2.distance) return p1;
    return p2;
}

Ray rayGen() {
    vec3 rayOrigin = scene.cameraPos.xyz;
    vec2 coord = vec2(vPosition.x * scene.aspectRatio, vPosition.y) / 2.0;
    vec3 rayDirection = (scene.viewInverse * vec4(normalize(vec3(coord, -1)), 0)).xyz;
    return Ray(rayOrigin, normalize(rayDirection));
}

Payload intersect(Ray ray, Sphere sphere) {
    Material mat;
    Payload payload = Payload(-1.0, vec3(0.0), mat);

    vec3 originToSphere = ray.origin - sphere.position;
    float p = dot(ray.direction, originToSphere);
    float q = dot(originToSphere, originToSphere) - (sphere.radius * sphere.radius);
    float d = (p * p) - q;

    if (d < 0.0) return payload;

    float dist = -p - sqrt(d);
    if (dist > 0.0) {
        vec3 point = ray.origin + ray.direction * dist;
        vec3 centreToPoint = sphere.position - point;

        payload.distance = dist;
        payload.normal = -normalize(centreToPoint);
        payload.material = sphere.material;
        return payload;
    }
    return payload;
}

Payload intersect(Ray ray, Triangle tri) {
    Material mat;
    Payload payload = Payload(-1.0, vec3(0.0), mat);

    vec3 v0 = tri.p1;
    vec3 v1 = tri.p2;
    vec3 v2 = tri.p3;

    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    vec3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);
    float epsilon = 1e-6;

    if (abs(a) < epsilon) return payload;

    float f = 1.0 / a;
    vec3 s = ray.origin - v0;
    float u = f * dot(s, h);

    if (u < 0.0 || u > 1.0) return payload;

    vec3 q = cross(s, edge1);
    float v = f * dot(ray.direction, q);

    if (v < 0.0 || u + v > 1.0) return payload;

    float t = f * dot(edge2, q);

    if (t > epsilon) {
        payload.distance = t;
        payload.material = tri.material;

        vec3 normal = normalize(cross(edge1, edge2));
        if (dot(ray.direction, normal) > 0.0) {
            normal = -normal;
        }

        payload.normal = normal;
        return payload;
    }
    return payload;
}

Payload intersect(Ray ray, Box box) {
    Material mat;
    Payload payload = Payload(-1.0, vec3(0.0), mat);

    vec3 minV = -vec3(0.5) + box.position;
    vec3 maxV = vec3(1.0) * box.dimensions - vec3(0.5) + box.position;

    vec3 tMin = (minV - ray.origin) / ray.direction;
    vec3 tMax = (maxV - ray.origin) / ray.direction;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);

    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);

    if (tNear > tFar || tNear < 0.0) return payload;

    vec3 intersectionPoint = ray.origin + ray.direction * tNear;
    vec3 normal = vec3(0.0);
    float epsilon = 1e-4;

    if (abs(intersectionPoint.x - minV.x) < epsilon) normal = vec3(-1, 0, 0);
    else if (abs(intersectionPoint.x - maxV.x) < epsilon) normal = vec3(1, 0, 0);
    else if (abs(intersectionPoint.y - minV.y) < epsilon) normal = vec3(0, -1, 0);
    else if (abs(intersectionPoint.y - maxV.y) < epsilon) normal = vec3(0, 1, 0);
    else if (abs(intersectionPoint.z - minV.z) < epsilon) normal = vec3(0, 0, -1);
    else if (abs(intersectionPoint.z - maxV.z) < epsilon) normal = vec3(0, 0, 1);

    payload.distance = tNear;
    payload.normal = normal;
    payload.material = box.material;

    return payload;
}

Payload closestHit(Ray ray) {
    Payload payload;
    payload.distance = -1.0;

    for (int i = 0; i < scene.sphereNum; i++) {
        payload = minPayload(payload, intersect(ray, sphereList[i]));
    }
    for (int i = 0; i < scene.boxNum; i++) {
        payload = minPayload(payload, intersect(ray, boxList[i]));
    }
    for (int i = 0; i < scene.triangleNum; i++) {
        payload = minPayload(payload, intersect(ray, triangleList[i]));
    }
    return payload;
}

vec3 cosineSampleHemisphere(vec3 n) {
    float u1 = rand01();
    float u2 = rand01();
    float r = sqrt(u1);
    float theta = 2.0 * PI * u2;

    vec3 local = vec3(r * cos(theta), r * sin(theta), sqrt(max(0.0, 1.0 - u1)));

    vec3 t = (abs(n.z) < 0.999) ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(t, n));
    vec3 bitangent = cross(n, tangent);

    return normalize(tangent * local.x + bitangent * local.y + n * local.z);
}

vec3 traceRay(Ray ray) {
    Payload payload;
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);

    int maxBounces = min(scene.bounces, 16); // Hard cap to prevent infinite TDR loops

    for (int bounce = 0; bounce < maxBounces; bounce++) {
        payload = closestHit(ray);

        if (payload.distance < 0.0) {
            radiance += throughput * texture(cubemap, ray.direction).xyz;
            break;
        }

        vec3 emission = payload.material.albedo * payload.material.emissive;
        radiance += throughput * emission;

        ray.origin += ray.direction * payload.distance;
        throughput *= payload.material.albedo;

        // Corrected reflection and diffuse bounce math
        vec3 diffuseDir = cosineSampleHemisphere(payload.normal);
        vec3 specularDir = reflect(ray.direction, payload.normal);

        // Mix between perfect specular reflection and diffuse based on roughness
        vec3 targetDir = mix(specularDir, diffuseDir, payload.material.roughness);

        ray.direction = normalize(targetDir);
        ray.origin += payload.normal * 0.001; // Offset to prevent self-intersection
    }

    return radiance;
}

out vec4 fragColor;

void main() {
    // Seed the PRNG safely using screen space and time
    rngState = uint(gl_FragCoord.x) * 1973u +
    uint(gl_FragCoord.y) * 9277u +
    uint(scene.frameCounter + 1.0) * 26699u + 1u;
    rngState = hash(rngState);

    Ray ray = rayGen();
    const int samples = 3;
    vec3 color = vec3(0.0);

    for (int i = 0; i < samples; i++) {
        Ray newRay = ray;

        // Anti-aliasing jitter
        float jitterX = (rand01() * 2.0 - 1.0) / 1000.0;
        float jitterY = (rand01() * 2.0 - 1.0) / 1000.0;
        newRay.direction = normalize(newRay.direction + vec3(jitterX, jitterY, 0.0));

        color += traceRay(newRay);
    }
    color /= float(samples);

    fragColor = vec4(color, 1.0);

    vec2 coords = vPosition / 2.0 + 0.5;
    vec4 p = texture(text, coords);

    if (!scene.reset) {
        fragColor = mix(p, fragColor, 1.0 / (scene.frameCounter + 1.0));
    }
}