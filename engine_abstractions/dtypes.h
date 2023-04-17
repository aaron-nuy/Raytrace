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


	struct Ray {
		Ray(vec3 d,vec3 o) :
			origin(o),
			direction(d)
		{}
		vec3 origin;
		vec3 direction;
	};

	struct Shape {

		
	public:
		/*
			Returns -1.f if no intersections
		*/
		vec3 position;
		Material material;

		Shape(vec3 pos, Material mat) :
			position(pos),
			material(mat)
		{}

		virtual GLfloat intersect(const Ray& ray) = 0;

	};

	struct Sphere : public Shape {
		GLfloat radius;

		Sphere(vec3 pos, GLfloat rad, Material mat) : 
			Shape(pos,mat),
			radius(rad)
		{}

		GLfloat intersect(const Ray& ray) override {

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

		Box(vec3 pos, vec3 dimens, Material mat) :
			Shape(pos, mat),
			dimensions(dimens)
		{}

		GLfloat intersect(const Ray& ray) override {
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

