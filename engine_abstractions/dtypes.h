#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"


namespace rtre {
    using glm::vec4;
    using glm::vec3;
    using glm::vec2;
    using glm::mat4;
    using glm::mat3;
    using glm::mat2;

    struct Material {
        vec3 albedo;
        GLfloat roughness;
        GLfloat metalic;
        GLfloat emissive;
        GLfloat specular;
    };

    struct GPUMaterial {
        alignas(16) vec3 albedo;
        float roughness;
        float metalic;
        float emissive;
        float specular;
    };

    struct GPUSphere {
        alignas(16) vec3 position;
        float radius;
        GPUMaterial material;
    };

    struct GPUBox {
        alignas(16) vec3 position;
        alignas(16) vec3 dimensions;
        GPUMaterial material;
    };

    struct GPUTriangle {
        alignas(16) vec3 p1;
        alignas(16) vec3 p2;
        alignas(16) vec3 p3;
        GPUMaterial material;
    };

    struct SceneData {
        mat4 viewInverse;
        vec4 cameraPos;
        float time;
        float aspectRatio;
        float frameCounter;
        int bounces;
        uint32_t sphereNum;
        uint32_t boxNum;
        uint32_t triangleNum;
        uint32_t state;
        int reset;
    };


    struct Ray {
        Ray(vec3 d, vec3 o) : origin(o),
                              direction(d)
        {
        }

        vec3 origin;
        vec3 direction;
    };

    struct Shape {
    public:
        /*
			Returns -1.f if no intersections
		*/
        Material material;

        Shape(Material mat) : material(mat)
        {
        }

        virtual GLfloat intersect(const Ray &ray) = 0;
    };

    struct Sphere : public Shape {
        GLfloat radius;
        vec3 position;

        Sphere(vec3 pos, GLfloat rad, Material mat) : Shape(mat),
                                                      position(pos),
                                                      radius(rad)
        {
        }

        GLfloat intersect(const Ray &ray) override
        {
            vec3 originToSphere = ray.origin - position;

            GLfloat p = glm::dot(ray.direction, originToSphere);
            GLfloat q = glm::dot(originToSphere, originToSphere) - (radius * radius);

            GLfloat d = (p * p) - q;
            if (d < 0.0)
                return -1.0;

            GLfloat dist = -p - glm::sqrt(d);

            if (dist > 0.0)
                return dist;


            return -1.0;
        }
    };

    struct Box : public Shape {
    public:
        vec3 dimensions;
        vec3 position;

        Box(vec3 pos, vec3 dimens, Material mat) : Shape(mat),
                                                   position(pos),
                                                   dimensions(dimens)
        {
        }

        GLfloat intersect(const Ray &ray) override
        {
            glm::vec3 min(0);
            glm::vec3 max(1);

            min = min - glm::vec3(0.5) + position;
            max = max * dimensions - glm::vec3(0.5) + position;

            vec3 tMin = (min - ray.origin) / ray.direction;
            vec3 tMax = (max - ray.origin) / ray.direction;
            vec3 t1 = glm::min(tMin, tMax);
            vec3 t2 = glm::max(tMin, tMax);
            float tNear = glm::max(glm::max(t1.x, t1.y), t1.z);
            float tFar = glm::min(glm::min(t2.x, t2.y), t2.z);

            if (tNear > tFar || tNear < 0.0)
                return -1.0;

            return tNear;
        }
    };

    struct Triangle : public Shape {
        vec3 points[3];

        Triangle(const vec3& p1, const vec3& p2, const vec3& p3, Material mat)
            : Shape(mat), points{p1, p2, p3} {}

        Triangle(const vec3 (&p)[3], Material mat)
            : Shape(mat), points{p[0], p[1], p[2]} {}


        GLfloat intersect(const Ray &ray) override
        {
            const float EPSILON = 0.0000001f;

            glm::vec3 v0 = points[0];
            glm::vec3 v1 = points[1];
            glm::vec3 v2 = points[2];

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;

            glm::vec3 h = glm::cross(ray.direction, edge2);
            float a = glm::dot(edge1, h);

            if (a > -EPSILON && a < EPSILON)
                return -1.0f;

            float f = 1.0f / a;
            glm::vec3 s = ray.origin - v0;
            float u = f * glm::dot(s, h);

            if (u < 0.0f || u > 1.0f)
                return -1.0f;

            glm::vec3 q = glm::cross(s, edge1);
            float v = f * glm::dot(ray.direction, q);

            if (v < 0.0f || u + v > 1.0f)
                return -1.0f;

            float t = f * glm::dot(edge2, q);

            if (t > EPSILON)
                return t;

            return -1.0f;
        }

    };

    class PointLight {
    public:
        vec3 position = vec3(0, 0, 0);

        vec3 diffuse = vec3(1, 1, 1);
        vec3 specular = vec3(1, 1, 1);

        GLfloat constant = 0.8;
        GLfloat linear = 0.02f;
        GLfloat quadratic = 0.032f;

        PointLight(vec3 pos, vec3 dif, vec3 spec = vec3(1.f, 1.f, 1.f),
                   GLfloat cons = 0.8, GLfloat lin = 0.02f, GLfloat quad = 0.032f);

        ~PointLight();
    };
}