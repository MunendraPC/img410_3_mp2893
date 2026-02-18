// headers
#include "raycast_lib.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <cstdint>
#include <cstdlib>

float dot3(const float a[3], const float b[3]){
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void normalize3(float v[3]){
    float len = std::sqrt(dot3(v, v));
    if(len > 1e-6f){
        v[0] /= len; v[1] /= len; v[2] /= len;
    }
}

uint8_t toByte(float x){
    if(x < 0.0f) x = 0.0f;
    if(x > 1.0f) x = 1.0f;
    int v = (int)std::lround(x * 255.0f);
    if(v < 0) v = 0;
    if(v > 255) v = 255;
    return (uint8_t)v;
}

// raycast hit sphere
bool hitSphere(const float O[3], const float D[3], sceneData* s, float &tHit){
    float C[3] = { s->position[0], s->position[1], s->position[2] };
    sphere* spherePtr = static_cast<sphere*>(s);
    float r = spherePtr->radius;

    float OC[3] = { O[0]-C[0], O[1]-C[1], O[2]-C[2] };

    float a = dot3(D, D);
    float b = 2.0f * dot3(OC, D);
    float c = dot3(OC, OC) - r*r;

    float disc = b*b - 4.0f*a*c;
    if(disc < 0.0f) return false;

    float sd = std::sqrt(disc);
    float t0 = (-b - sd) / (2.0f*a);
    float t1 = (-b + sd) / (2.0f*a);

    const float EPS = 1e-6f;
    if(t0 > EPS){ tHit = t0; return true; }
    if(t1 > EPS){ tHit = t1; return true; }
    return false;
}

// raycast hit plane
bool hitPlane(const float O[3], const float D[3], sceneData* p, float &tHit){
    plane* planePtr = static_cast<plane*>(p);
    float N[3] = { planePtr->normal[0], planePtr->normal[1], planePtr->normal[2] };
    normalize3(N);

    float P0[3] = { p->position[0], p->position[1], p->position[2] };

    float denom = dot3(N, D);
    const float EPS = 1e-6f;
    if(std::fabs(denom) < EPS) return false;

    float P0O[3] = { P0[0]-O[0], P0[1]-O[1], P0[2]-O[2] };
    float t = dot3(N, P0O) / denom;
    if(t <= EPS) return false;

    tHit = t;
    return true;
}

// read scene
bool readScene(char file[], sceneData** Objects, sceneData* camera, int* objCount)
{
    std::ifstream scene(file);
    if(!scene.is_open()){
        std::cerr << "Error: Could not open file " << file << "\n";
        return false;
    }

    // defaults for camera
    camera->cam_width = 0.0f;
    camera->cam_height = 0.0f;
    camera->c_diff[0]=camera->c_diff[1]=camera->c_diff[2]=0.0f;
    camera->position[0]=camera->position[1]=camera->position[2]=0.0f;
    camera->type = OBJ_CAMERA;

    std::string magic;
    scene >> magic;
    if(magic != "img410scene"){
        std::cerr << "Error: Only img410scene format supported\n";
        return false;
    }

    std::string elem;
    while(scene >> elem){
        if(elem == "end") break;

        // camera
        if(elem == "camera"){
            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                if(!property.empty() && property.back() == ';') property.pop_back();

                if(property == "width:"){
                    scene >> camera->cam_width;
                } else if(property == "height:"){
                    scene >> camera->cam_height;
                }

                if(endObj) break;
            }
        }

        // sphere
        else if(elem == "sphere"){
            if(*objCount >= 128){
                std::cerr << "Error: too many objects\n";
                return false;
            }

            sceneData* s = new sphere();
            sphere* spherePtr = static_cast<sphere*>(s);
            // defaults if elements are missing
            spherePtr->c_diff[0]=spherePtr->c_diff[1]=spherePtr->c_diff[2]=0.0f;
            spherePtr->position[0]=spherePtr->position[1]=spherePtr->position[2]=0.0f;  
            spherePtr->radius = 0.0f;
            spherePtr->type = OBJ_SPHERE;

            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                if(!property.empty() && property.back() == ';') property.pop_back();

                if(property == "c_diff:"){
                    scene >> spherePtr->c_diff[0] >> spherePtr->c_diff[1] >> spherePtr->c_diff[2];
                } else if(property == "position:"){
                    scene >> spherePtr->position[0] >> spherePtr->position[1] >> spherePtr->position[2];
                } else if(property == "radius:"){
                    scene >> spherePtr->radius;
                }

                if(endObj) break;
            }

            Objects[*objCount] = s;
            (*objCount)++;
        }

        // plane
        else if(elem == "plane"){
            if(*objCount >= 128){
                std::cerr << "Error: too many objects\n";
                return false;
            }

            sceneData* p = new plane();
            plane* planePtr = static_cast<plane*>(p);
            // defaults if elements are missing
            planePtr->c_diff[0]=planePtr->c_diff[1]=planePtr->c_diff[2]=0.0f;
            planePtr->position[0]=planePtr->position[1]=planePtr->position[2]=0.0f;
            planePtr->normal[0]=planePtr->normal[1]=planePtr->normal[2]=0.0f;
            planePtr->type = OBJ_PLANE;

            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                if(!property.empty() && property.back() == ';') property.pop_back();

                if(property == "c_diff:"){
                    scene >> planePtr->c_diff[0] >> planePtr->c_diff[1] >> planePtr->c_diff[2];
                } else if(property == "position:"){
                    scene >> planePtr->position[0] >> planePtr->position[1] >> planePtr->position[2];
                } else if(property == "normal:"){
                    scene >> planePtr->normal[0] >> planePtr->normal[1] >> planePtr->normal[2];
                    normalize3(planePtr->normal);
                }

                if(endObj) break;
            }

            Objects[*objCount] = p;
            (*objCount)++;
        }

        else {
            // do nothing if unrecognized or giberish
        }
    }

    return true;
}

bool writeppm(const char* outFile, int Wid, int Height, const uint8_t* pix){
    std::ofstream out(outFile);
    if(!out.is_open()) return false;

    out << "P3\n" << Wid << " " << Height << "\n255\n";
    for(int j=0;j<Height;j++){
        for(int i=0;i<Wid;i++){
            int idx = (j*Wid + i) * 3;
            out << (int)pix[idx] << " " << (int)pix[idx+1] << " " << (int)pix[idx+2] << "\n";
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    if(argc != 5){
        std::printf("Usage: raycast width height input.scene output.ppm\n\n");
        return 1;
    }

    int Wid = std::atoi(argv[1]);
    int Height = std::atoi(argv[2]);

    sceneData** objects = new sceneData*[128];
    int objCount = 0;
    sceneData camera;

    if(!readScene(argv[3], objects, &camera, &objCount)){
        return 1;
    }

    // confirm the cam wid and height are not zero
    if(camera.cam_width == 0.0f)  camera.cam_width = 1.0f;
    if(camera.cam_height == 0.0f) camera.cam_height = 1.0f;

    // pixmap for ppm file
    uint8_t* pix = new uint8_t[Wid * Height * 3];

    // camera origin
    float Origin[3] = {0.0f, 0.0f, 0.0f};

    // render scene
    for(int j=0;j<Height;j++){
        for(int i=0;i<Wid;i++){
            float x = (i + 0.5f) / (float)Wid;
            float y= (j + 0.5f) / (float)Height;

            float Rd[3];
            Rd[0] = (-camera.cam_width * 0.5f) + x * camera.cam_width;
            Rd[1] = (camera.cam_height * 0.5f)  - y * camera.cam_height;
            Rd[2] = -1.0f;
            normalize3(Rd);

            float min_t = std::numeric_limits<float>::infinity();
            float bestColor[3] = {0.0f, 0.0f, 0.0f};
            bool hit = false;

            // intersection by object type
            for(int s=0;s<objCount;s++){
                float tHit;
                if(objects[s]->type == OBJ_SPHERE){
                    if(hitSphere(Origin, Rd, objects[s], tHit) && tHit < min_t){
                        min_t = tHit;
                        bestColor[0] = objects[s]->c_diff[0];
                        bestColor[1] = objects[s]->c_diff[1];
                        bestColor[2] = objects[s]->c_diff[2];
                        hit = true;
                    }
                } else if(objects[s]->type == OBJ_PLANE){
                    if(hitPlane(Origin, Rd, objects[s], tHit) && tHit < min_t){
                        min_t = tHit;
                        bestColor[0] = objects[s]->c_diff[0];
                        bestColor[1] = objects[s]->c_diff[1];
                        bestColor[2] = objects[s]->c_diff[2];
                        hit = true;
                    }
                }
            }

            int idx = (j*Wid + i) * 3;
            if(hit){
                pix[idx+0] = toByte(bestColor[0]);
                pix[idx+1] = toByte(bestColor[1]);
                pix[idx+2] = toByte(bestColor[2]);
            } else {
                pix[idx+0] = 0;
                pix[idx+1] = 0;
                pix[idx+2] = 0;
            }
        }
    }

    // write ppm image and clean up memory
    if(!writeppm(argv[4], Wid, Height, pix)){
        std::cerr << "Error: Could not write output file " << argv[4] << "\n";
        delete[] pix;
        delete[] objects;
        return 1;
    }

    delete[] pix;
    for(int s=0;s<objCount;s++) delete objects[s];
    delete[] objects;

    return 0;
}
