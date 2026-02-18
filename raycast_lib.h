#ifndef RAYCAST_LIB_H
#define RAYCAST_LIB_H

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <cstdint>
#include <cstdlib>

#define MAX_OBJECTS 128
#define EPSILON 1e-6f

// objects
enum ObjectType {
    OBJ_CAMERA,
    OBJ_SPHERE,
    OBJ_PLANE
};


typedef struct sceneData {
    ObjectType type;

    float c_diff[3];     
    float position[3];   

    // camera properties
    float cam_width;
    float cam_height;

} sceneData;


typedef struct sphere : sceneData {
    float radius;
} sphere;


typedef struct plane : sceneData {
    float normal[3];
} plane;

#endif // RAYCAST_LIB_H
