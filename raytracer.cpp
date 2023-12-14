/**
 * @name Simple Raytracer
 * @author Jiahui Hu
*/

#define GLM_ENABLE_EXPERIMENTAL
#define RAYTRACER_MAX_DEPTH 4

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ppm.cpp"
#include "raytracer.hpp"

using namespace glm;
using namespace std;

vec3 raytrace(Ray ray, int times, Scene& scene);
vec3 shadowRay(Light light, vec3 pos, Scene& scene);
bool intersectSphere(Ray ray, Sphere sphere, vec3 &intersection, vec3 &normal);
Scene fileToScene(ifstream &file);
void handleScene(Scene scene);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Please provide a test case as argument\n" << endl;
        return 1;
    }

    ifstream file("./" + string(argv[1]));

    if (!file.is_open()) {
        cerr << "Failed to open file: " << "./" + string(argv[1]) << "\n";
        return 1;
    }
    
    Scene scene = fileToScene(file);
    handleScene(scene);
    file.close();
    
    /*
    // Test all cases
    std::string filenames[] = { "testAmbient.txt", "testBackground.txt", "testBehind.txt", "testDiffuse.txt", "testIllum.txt", "testImgPlane.txt", "testIntersection.txt", "testReflection.txt", "testSample.txt", "testShadow.txt", "testSpecular.txt" };
    
    for (string filename : filenames)
    {
        std::ifstream file("./" + filename);
        Scene scene = fileToScene(file);
        handleScene(scene);
        file.close();
    }
    */

    return 0;
}

/**
 * Parse a test case file to a scene.
*/
Scene fileToScene(ifstream &file)
{
    Scene scene;
    std::string line;
    while (std::getline(file, line))
    {
        istringstream iss(line);
        string item;
        iss >> item;
        if (item == "NEAR")
        {
            iss >> scene.near;
        }
        else if (item == "LEFT")
        {
            iss >> scene.left;
        }
        else if (item == "RIGHT")
        {
            iss >> scene.right;
        }
        else if (item == "BOTTOM")
        {
            iss >> scene.bottom;
        }
        else if (item == "TOP")
        {
            iss >> scene.top;
        }
        else if (item == "RES")
        {
            iss >> scene.nColumns >> scene.nRows;
        }
        else if (item == "SPHERE")
        {
            Sphere sphere;
            iss
            >> sphere.name
            >> sphere.pos.x >> sphere.pos.y >> sphere.pos.z
            >> sphere.scl.x >> sphere.scl.y >> sphere.scl.z
            >> sphere.color.x >> sphere.color.y >> sphere.color.z
            >> sphere.k_a >> sphere.k_d >> sphere.k_s >> sphere.k_r
            >> sphere.n;
            //cout << "Sphere scale: " << sphere.scl.x << sphere.scl.y << sphere.scl.z << endl;
            scene.spheres.push_back(sphere);            
        }
        else if (item == "LIGHT")
        {
            Light light;
            iss
            >> light.name
            >> light.pos.x >> light.pos.y >> light.pos.z
            >> light.color.x >> light.color.y >> light.color.z;
            scene.lights.push_back(light);
        }
        else if (item == "BACK")
        {
            iss >> scene.back.x >> scene.back.y >> scene.back.z;
        }
        else if (item == "AMBIENT")
        {
            iss >> scene.ambient.x >> scene.ambient.y >> scene.ambient.z;
        }
        else if (item == "OUTPUT")
        {
            iss >> scene.output;
        }
    }

    return scene;
}

/**
 * Generate a ppm image based on the provided scene.
*/
void handleScene(Scene scene)
{
    // Setup metric data
    vec3 topLeft = vec3(scene.left, scene.top, -scene.near);
    float unitWidth = (scene.right - scene.left) / scene.nColumns;
    float unitHeight = (scene.top - scene.bottom) / scene.nRows;

    // Setup pixels
    unsigned char pixels[scene.nColumns * scene.nRows * 3];
    for (int i = 0; i < scene.nColumns * scene.nRows * 3; i++)
        pixels[i] = 1;

    // Iterate each screen pixel for raytracing
    for (int j = 0; j < scene.nRows; j++)
    {
        for (int i = 0; i < scene.nColumns; i++)
        {
            // Get screen coordinates
            vec3 screenPoint = topLeft + vec3((i + 0.5f) * unitWidth, (j + 0.5f) * -unitHeight, 0);
            
            // Cast a ray
            Ray ray;
            ray.init(vec3(0, 0, 0), screenPoint - vec3(0, 0, 0));
            vec3 color = saturate(raytrace(ray, RAYTRACER_MAX_DEPTH, scene));

            // Set colors
            int index = (j * scene.nColumns + i) * 3;
            pixels[index] = (int)(color.x * 255);
            pixels[index + 1] = (int)(color.y * 255);
            pixels[index + 2] = (int)(color.z * 255);
        }
    }
    
    // Generate image
    save_imageP6(scene.nColumns, scene.nRows, scene.output, pixels);
}   

/**
 * Cast a show ray from a point to a light in the scene
*/
vec3 shadowRay(Light light, vec3 pos, Scene& scene)
{
    // Setup ray: point to light
    Ray ray;
    ray.init(pos, light.pos - pos);

    // Return black when hitting any obstacles
    for (Sphere sphere : scene.spheres)
    {
        vec3 intersection;
        vec3 normal;
        if (intersectSphere(ray, sphere, intersection, normal))
            return vec3(0, 0, 0);
    }

    // Return light color
    return light.color;
}

/**
 * Cast a ray in the scene
*/
vec3 raytrace(Ray ray, int rayDepth, Scene& scene)
{
    // Setup hit data
    vec3 hitPoint;
    vec3 hitNormal;
    float hitDist = MAXFLOAT;
    Sphere hitSphere;

    // Find the closest hit
    for (Sphere sphere : scene.spheres) {
        vec3 intersection;
        vec3 normal;
        if (intersectSphere(ray, sphere, intersection, normal))
        {
            float dist = distance(intersection, ray.start);
            if (dist < hitDist)
            {
                hitPoint = intersection;
                hitNormal = normal;
                hitDist = dist;
                hitSphere = sphere;
            }
        }
    }

    // Return default color when hitting nothing: screen ray -> background color, relfected ray -> black
    if (hitDist == MAXFLOAT)
        return rayDepth < RAYTRACER_MAX_DEPTH ? vec3(0) : scene.back;


    // Setup local color
    vec3 colorLocal = vec3(0);

    // Get ambient color
    colorLocal += hitSphere.k_a * scene.ambient * hitSphere.color;
    
    // Get diffuse and specular colors
    for (Light light : scene.lights)
    {   
        vec3 lightColor = shadowRay(light, hitPoint, scene);
        vec3 lightVec = normalize(light.pos - hitPoint);

        colorLocal += hitSphere.k_d * lightColor * dot(hitNormal, lightVec) * hitSphere.color;

        double rDotV = dot(normalize(reflect(hitPoint - light.pos, hitNormal)), -ray.direction);
        colorLocal += hitSphere.k_s * lightColor * (float)pow(rDotV, hitSphere.n);
    }

    // Setup reflection color
    vec3 colorReflected = vec3(0);

    // Cast a reflected ray when having depth
    if (rayDepth > 0)
    {
        Ray rayReflected;
        vec3 direction = reflect(ray.direction, hitNormal);
        rayReflected.init(hitPoint, direction);

        colorReflected = hitSphere.k_r * raytrace(rayReflected, rayDepth - 1, scene);
    }
    
    // Sum up colors
    vec3 color = colorLocal + colorReflected;

    // TEST: test normal
    //color = dot(hitNormal, normalize(-hitPoint)) * vec3(1);

    return color;
}

/**
 * Get the intersection data between a ray and a sphere
*/
bool intersectSphere(Ray ray, Sphere sphere, vec3 &intersection, vec3 &normal) {

    // Transform ray to the sphere coordinate system
    vec3 invScale = vec3(1.0 / sphere.scl.x, 1.0 / sphere.scl.y, 1.0 / sphere.scl.z);
    vec3 localRayStart = (ray.start - sphere.pos) * invScale;
    vec3 localRayDir = ray.direction * invScale;

    // Get the intersection in the sphere system
    vec3 localIntersection, localNormal;
    bool hit = intersectRaySphere(localRayStart, normalize(localRayDir), vec3(0, 0, 0), 1.0, localIntersection, localNormal);

    // Update data when hit
    if (hit) {
        // Transform local data to world
        intersection = localIntersection * sphere.scl + sphere.pos;

        normal = normalize(localNormal * invScale);
        //normal = normalize((localNormal * invScale) / dot(invScale, invScale));
        //normal = normalize(intersection - sphere.pos);

        // Add an offset to avoid self shadow
        intersection += normal * 0.0001f;
    }

    return hit;
}
