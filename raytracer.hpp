/**
 * @name Simple Raytracer
 * @author Jiahui Hu
*/

#include "glm/ext.hpp"
#include "glm/glm.hpp"
#include <vector>

using namespace glm;
using namespace std;

/**
 * A 3D sphere with the rendering data.
*/
struct Sphere
{
    char name[20];
    glm::vec3 pos;
    glm::vec3 scl;
    glm::vec3 color;
    float k_a;
    float k_d;
    float k_s;
    float k_r;
    int n;
};

/**
 * A 3D point light with the rendering data.
*/
struct Light
{
    char name[20];
    glm::vec3 pos;
    glm::vec3 color;
};

/**
 * A scene containing all data in a test case.
*/
struct Scene
{
    float near;
    float left;
    float right;
    float bottom;
    float top;
    int nColumns;
    int nRows;
    std::vector<Sphere> spheres;
    std::vector<Light> lights;
    glm::vec3 back;
    glm::vec3 ambient;
    char output[20];
};

/**
 * A 3D ray represented by a start point and a direction.
*/
struct Ray
{
    glm::vec3 start;
    glm::vec3 direction;

    /**
     * Initialize the ray. Normalize the given direction.
    */
    void init(glm::vec3 start_pos, glm::vec3 ray_direction)
    {
        start = start_pos;
        direction = glm::normalize(ray_direction);
    }
};


