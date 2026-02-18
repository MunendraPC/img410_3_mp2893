#ifndef RAYCAST_LIB_H
#define RAYCAST_LIB_H

#include <stdio.h>
#include <stdint.h>
#include <string>
#include <stdbool.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>

typedef struct sceneData{
    std::string magicNum;

    float c_diff[3];      // diffuse color
    float x, y, z;        // position for sphere/plane

    float cam_width;      // camera width:
    float cam_height;     // camera height:
} sceneData;

typedef struct sphere : sceneData{
    float radius;
} sphere;

typedef struct plane : sceneData{
    float normal[3];      // MUST be 3D
} plane;


// vector library (as you already had)
void  v3_from_points(float *dst, float *a, float *b);
void  v3_add(float *dst, float *a, float *b);
void  v3_subtract(float *dst, float *a, float *b);
float v3_dot_product(float *a, float *b);
void  v3_cross_product(float *dst, float *a, float *b);
void  v3_scale(float *dst, float s);
float v3_angle(float *a, float *b);
float v3_angle_quick(float *a, float *b);
void  v3_reflect(float *dst, float *v, float *n);
float v3_length(float *a);
void  v3_normalize(float *dst, float *a);

bool  v3_equals(float *a, float *b, float tolerance);

#endif // RAYCAST_LIB_H
