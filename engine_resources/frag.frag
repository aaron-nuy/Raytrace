#version 410 core
#define PI 3.14159265359

#define ARRAY_SIZE 64

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

struct Ray{
	vec3 origin;
	vec3 direction;
};

struct Payload{
	float distance;
	vec3 normal;
	Material material;
};

uniform float aspec;
uniform float time;
uniform float counter;
uniform mat4 matrix;
uniform vec3 cameraPos;
uniform samplerCube cubemap;
uniform uint state;
uniform Sphere sphereList[ARRAY_SIZE];
uniform Box boxList[ARRAY_SIZE];
uniform int bounce;
uniform uint sphereNum;
uniform uint boxNum;
uniform sampler2D text;

uniform bool reset;

uint unDState;
uint dState = 22;
const float inf = 3.402823466e38;

Payload min(Payload p1, Payload p2){
	if(p1.distance < 0.0 )
		return p2;
	if(p2.distance < 0.0 )
		return p1;
	if(p1.distance < p2.distance)
		return p1;		
	return p2;
}


vec3 powV(vec3 vector, float power) {
	return vec3( pow(vector.x,power) ,pow(vector.y,power) ,pow(vector.z,power));
}	

// fixed through space
float rand() {
	unDState = unDState* 747796405+2891336453;
	uint result = ((unDState >> ((unDState >> 28) + 4 )) ^ unDState) * 277803737;
	result = (result >> 22) ^ result;
	return result / 4294967295.0;
}

// fixed through space
vec3 randVec() { 
	return vec3(rand(),rand(),rand());
}

// random through time and space
float randPixel() {
    return fract(sin(dot(vPosition,
                         vec2(12.9898,78.233)))*
        43758.5453123 + fract(sin(time/100)+ fract(rand()) ) );
}

// random through time and space
vec3 randVecPixel() {
    return vec3(
	fract(sin(dot(vPosition.yx, vec2(12.3898,78.233)))*43758.5453123 + fract(time) + fract(rand()) ),
	fract(sin(dot(vPosition   , vec2(12.9498,78.233)))*43758.5453123 + fract(time) + fract(rand()) ),
	fract(sin(dot(vPosition.xy, vec2(12.1828,78.233)))*43758.5453123 + fract(time) + fract(rand()) )
	);
}

// Fixed through time
float randD() {
	return fract(sin(dot(vPosition, vec2(12.3898,78.233)))*43758.5453123 );
}

float rand(float x, float t) {
	return fract(sin(dot(vec2(x,t), vec2(12.9898, 78.233))) * 43758.5453);
}

// Fixed through time
vec3 randVecD() {
    return vec3(
	fract(sin(dot(vPosition, vec2(12.3898,78.233)))*43758.5453123 ),
	fract(sin(dot(vPosition, vec2(12.9498,78.233)))*43758.5453123 ),
	fract(sin(dot(vPosition, vec2(12.1828,78.233)))*43758.5453123 )
	);
}

//if breaks use the one over it 
// Fixed through time and space
float randDST() {
	dState = dState* 747796405+2891336453;
	uint result = ((dState >> ((dState >> 28) + 4 )) ^ dState) * 277803737;
	result = (result >> 22) ^ result;
	return result / 4294967295.0 ;
}

// Fixed through time and space
vec3 randVecDST(){
	return vec3(
		randD(),
		randD(),
		randD()
	);
}

vec3 randomSphere() {
    float theta = randDST() * 2.0 * 3.14;
    float x = randDST()*2.0 - 1.0;
    float s = sqrt(1.0 - x * x);
	return vec3(
    x,
    s * cos(theta),
    s * sin(theta)
	);
}


Ray rayGen() {
	vec3 rayOrigin = cameraPos;
	vec2 coord = vec2(vPosition.x*aspec,vPosition.y)/2.0;
	vec3 rayDirection = (matrix * vec4(normalize(vec3(coord,-1)),0)).xyz;


	return Ray(rayOrigin,rayDirection);
}

Payload intersect(Ray ray, Sphere sphere) {
	
	Material mat;
	Payload payload = Payload( -1.0, vec3(0.0), mat );

    vec3 originToSphere = ray.origin - sphere.position;

    float p = dot(ray.direction, originToSphere);
    float q = dot(originToSphere, originToSphere) - (sphere.radius * sphere.radius);

    float d = (p * p) - q;
    if (d < 0.0)
        return payload;

    float dist = -p - sqrt(d);

	if( dist > 0.0) {
		vec3 point = ray.origin + ray.direction * dist;
		vec3 centreToPoint = sphere.position - point;

		payload.distance = dist;
		payload.normal = -centreToPoint/sphere.radius;
		payload.material = sphere.material;

		return payload;
	}

	return payload;
}

Payload intersect(Ray ray, Box box){

	Material mat;
	Payload payload = Payload( -1.0,vec3(0.0),mat);
	
	vec3 minV = -vec3(0.5,0.5,0.5) + box.position;
	vec3 maxV = vec3(1,1,1) * box.dimensions - vec3(0.5,0.5,0.5) + box.position;


	vec3 tMin = (minV - ray.origin) / ray.direction;
    vec3 tMax = (maxV - ray.origin) / ray.direction;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
	
	if( tNear > tFar || tNear < 0.0 )
		return payload;

	vec3 intersectionPoint = ray.origin + ray.direction * tNear;

	vec3 normal;
	float epsilon = 1e-4;
    if (abs(intersectionPoint.x - minV.x) < epsilon) 
		normal = vec3(-1, 0, 0);
    else if (abs(intersectionPoint.x - maxV.x) < epsilon) 
		normal = vec3(1, 0, 0);
    else if (abs(intersectionPoint.y - minV.y) < epsilon)	
		normal = vec3(0, -1, 0);
    else if (abs(intersectionPoint.y - maxV.y) < epsilon) 
		normal = vec3(0, 1, 0);
    else if (abs(intersectionPoint.z - minV.z) < epsilon) 
		normal = vec3(0, 0, -1);
    else if (abs(intersectionPoint.z - maxV.z) < epsilon) 
		normal = vec3(0, 0, 1);

	payload.distance = tNear;
	payload.normal = normal;
	payload.material = box.material;

	return payload;
}

float lightIntersect(Ray ray, Sphere sphere) {
    vec3 originToSphere = ray.origin - sphere.position;

    float p = dot(ray.direction, originToSphere);
    float q = dot(originToSphere, originToSphere) - (sphere.radius * sphere.radius);

    float d = (p * p) - q;
    if (d < 0.0)
        return inf;

    float near = -p - sqrt(d);
    float far = -p + sqrt(d);

	if(abs(near) < abs(far))
		return near;
	return far;

}

float lightIntersect(Ray ray, Box box) {
	vec3 minV = -vec3(0.5,0.5,0.5) + box.position;
	vec3 maxV = vec3(1,1,1) * box.dimensions - vec3(0.5,0.5,0.5) + box.position;


	vec3 tMin = (minV - ray.origin) / ray.direction;
    vec3 tMax = (maxV - ray.origin) / ray.direction;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
	
	if( tNear > tFar )
		return inf;


	if(abs(tNear) < abs(tFar))
		return tNear;
	return tFar;
}


Payload closestHit(Ray ray){
	Payload payload;
	payload.distance = -1;

	for(int i = 0; i < sphereNum; i++) {
		payload = min(payload,intersect(ray,sphereList[i]));
	}

	for(int i = 0; i < boxNum; i++) {
		payload = min(payload,intersect(ray,boxList[i]));
	}

	return payload; 
}

vec3 calcLight(vec3 point,vec3 normal){
	return vec3(0);
}

vec3 miss(Ray ray){
	//return vec3(0.005);
	return texture(cubemap, ray.direction).xyz;
}

vec3 computeColor(Material mat, Ray ray, vec3 normal){
	return vec3(1.0);
}

vec3 reflect(vec3 direction, vec3 normal, float specular) {
    return normalize(direction -2 * normal* specular  * dot(direction,normal));
}


vec3 traceRay(Ray ray){

	Payload payload;
	vec3 color = vec3(1.0);
	unDState = state;
	vec3 firstHit;
	for(int i = 0; i<bounce; i++) {
		payload = closestHit(ray);
		float metalic = payload.material.metalic;
		vec3 albedo = payload.material.albedo;

		if(payload.distance < 0.0) {
			return color*miss(ray);
		}
		ray.origin += ray.direction*payload.distance;
		color = color*albedo*(1+payload.material.emissive*3);
		vec3 dir = normalize(randVecPixel());
		vec3 reflected_direction = reflect( ray.direction, payload.normal, payload.material.specular);
		ray.direction = mix( reflected_direction,dir*sign(dot(dir,payload.normal)), payload.material.roughness  );
		ray.origin += ray.direction*0.00001;
	}

	return color;
};

void main() {
	Ray ray = rayGen();
	const int samples = 3;
	vec3 color = vec3(0);

	for(int i = 1; i <=samples; i++) {
		Ray newRay = ray;
		newRay.direction = normalize(newRay.direction + vec3(randDST()) /1000.0);
		color += traceRay(newRay);
	}
	color = color/float(samples);

	gl_FragColor = vec4(color,1.0);


	vec2 coords = vPosition / 2.0 + 0.5;
	vec4 p = texture( text , coords);
	if(!reset)
		gl_FragColor = mix(p,gl_FragColor,1/(counter));


}