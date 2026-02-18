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

// function prototypes

float dot3(const float a[3], const float b[3]);

void normalize3(float v[3]);

uint8_t toByte(float x);

bool hitSphere(const float O[3], const float D[3], const sphere* s, float &tHit);

bool hitPlane(const float O[3], const float D[3], const plane* p, float &tHit);

bool readScene(char file[], sceneData* camera, sphere* spheres[], int* sphereCount, plane* planes[], int* planeCount);

bool writeppm(const char* outFile, int Wid, int Height, const uint8_t* pix);

#endif // RAYCAST_LIB_H
