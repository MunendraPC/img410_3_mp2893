// ray_casting.cpp  (FULL DROP-IN that matches the header your compiler is using)
//
// Your errors show:
// - sphere/plane do NOT have x,y,z  -> they likely have position[3]
// - sceneData does NOT have magicNum -> we read magic into a local std::string
//
// Assumes raycast_lib.h contains at least:
//   sceneData: float c_diff[3]; float position[3]; float cam_width; float cam_height;
//   sphere: float radius;
//   plane:  float normal[3];

#include "raycast_lib.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <cstdint>
#include <cstdlib>

// ----------------- small helpers -----------------
static void strip_semicolon(std::string &t){
    if(!t.empty() && t.back() == ';') t.pop_back();
}

static float dot3(const float a[3], const float b[3]){
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

static void normalize3(float v[3]){
    float len = std::sqrt(dot3(v, v));
    if(len > 1e-6f){
        v[0] /= len; v[1] /= len; v[2] /= len;
    }
}

static uint8_t to_byte(float x){
    if(x < 0.0f) x = 0.0f;
    if(x > 1.0f) x = 1.0f;
    int v = (int)std::lround(x * 255.0f);
    if(v < 0) v = 0;
    if(v > 255) v = 255;
    return (uint8_t)v;
}

// ----------------- intersections -----------------
static bool hit_sphere(const float O[3], const float D[3], const sphere* s, float &tHit){
    float C[3] = { s->position[0], s->position[1], s->position[2] };
    float r = s->radius;

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

static bool hit_plane(const float O[3], const float D[3], const plane* p, float &tHit){
    float N[3] = { p->normal[0], p->normal[1], p->normal[2] };
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

// ----------------- scene reading -----------------
bool readscene(char file[],
               sceneData* camera,
               sphere* spheres[], int* sphereCount,
               plane* planes[], int* planeCount)
{
    std::ifstream scene(file);
    if(!scene.is_open()){
        std::cerr << "Error: Could not open file " << file << "\n";
        return false;
    }

    // defaults
    camera->cam_width = 0.0f;
    camera->cam_height = 0.0f;
    camera->c_diff[0]=camera->c_diff[1]=camera->c_diff[2]=0.0f;
    camera->position[0]=camera->position[1]=camera->position[2]=0.0f;

    *sphereCount = 0;
    *planeCount = 0;

    // magic (do NOT store in sceneData; your header doesn't have magicNum)
    std::string magic;
    scene >> magic;
    if(magic != "img410scene"){
        std::cerr << "Error: Only img410scene format supported\n";
        return false;
    }

    std::string tok;
    while(scene >> tok){
        if(tok == "end") break;

        // CAMERA
        if(tok == "camera"){
            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                strip_semicolon(property);

                if(property == "width:"){
                    scene >> camera->cam_width;
                } else if(property == "height:"){
                    scene >> camera->cam_height;
                }

                if(endObj) break;
            }
        }

        // SPHERE
        else if(tok == "sphere"){
            if(*sphereCount >= 128){
                std::cerr << "Error: too many spheres\n";
                return false;
            }

            sphere* s = new sphere();
            // defaults
            s->c_diff[0]=s->c_diff[1]=s->c_diff[2]=0.0f;
            s->position[0]=s->position[1]=s->position[2]=0.0f;
            s->radius=0.0f;

            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                strip_semicolon(property);

                if(property == "c_diff:"){
                    scene >> s->c_diff[0] >> s->c_diff[1] >> s->c_diff[2];
                } else if(property == "position:"){
                    scene >> s->position[0] >> s->position[1] >> s->position[2];
                } else if(property == "radius:"){
                    scene >> s->radius;
                }

                if(endObj) break;
            }

            spheres[*sphereCount] = s;
            (*sphereCount)++;
        }

        // PLANE
        else if(tok == "plane"){
            if(*planeCount >= 128){
                std::cerr << "Error: too many planes\n";
                return false;
            }

            plane* p = new plane();
            // defaults
            p->c_diff[0]=p->c_diff[1]=p->c_diff[2]=0.0f;
            p->position[0]=p->position[1]=p->position[2]=0.0f;
            p->normal[0]=p->normal[1]=p->normal[2]=0.0f;

            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                strip_semicolon(property);

                if(property == "c_diff:"){
                    scene >> p->c_diff[0] >> p->c_diff[1] >> p->c_diff[2];
                } else if(property == "position:"){
                    scene >> p->position[0] >> p->position[1] >> p->position[2];
                } else if(property == "normal:"){
                    scene >> p->normal[0] >> p->normal[1] >> p->normal[2];
                    normalize3(p->normal);
                }

                if(endObj) break;
            }

            planes[*planeCount] = p;
            (*planeCount)++;
        }

        // unknown token: ignore
        else {
            // do nothing
        }
    }

    return true;
}

// ----------------- ppm writer -----------------
static bool write_ppm_p3(const char* outFile, int W, int H, const uint8_t* pix){
    std::ofstream out(outFile);
    if(!out.is_open()) return false;

    out << "P3\n" << W << " " << H << "\n255\n";
    for(int j=0;j<H;j++){
        for(int i=0;i<W;i++){
            int idx = (j*W + i) * 3;
            out << (int)pix[idx] << " " << (int)pix[idx+1] << " " << (int)pix[idx+2] << "\n";
        }
    }
    return true;
}

// ----------------- main -----------------
int main(int argc, char *argv[])
{
    if(argc != 5){
        std::printf("Usage: raycast width height input.scene output.ppm\n\n");
        return 1;
    }

    int W = std::atoi(argv[1]);
    int H = std::atoi(argv[2]);

    sceneData camera;
    sphere* spheres[128];
    plane*  planes[128];
    int sphereCount = 0;
    int planeCount = 0;

    if(!readscene(argv[3], &camera, spheres, &sphereCount, planes, &planeCount)){
        return 1;
    }

    if(camera.cam_width == 0.0f)  camera.cam_width = 1.0f;
    if(camera.cam_height == 0.0f) camera.cam_height = 1.0f;

    uint8_t* pix = new uint8_t[W * H * 3];

    // camera origin and view-plane
    float O[3] = {0.0f, 0.0f, 0.0f};
    float left = -camera.cam_width * 0.5f;
    float top  =  camera.cam_height * 0.5f;

    for(int j=0;j<H;j++){
        for(int i=0;i<W;i++){
            float u = (i + 0.5f) / (float)W;
            float v = (j + 0.5f) / (float)H;

            float D[3];
            D[0] = left + u * camera.cam_width;
            D[1] = top  - v * camera.cam_height;
            D[2] = -1.0f;
            normalize3(D);

            float bestT = std::numeric_limits<float>::infinity();
            float bestColor[3] = {0.0f, 0.0f, 0.0f};
            bool hit = false;

            for(int s=0;s<sphereCount;s++){
                float tHit;
                if(hit_sphere(O, D, spheres[s], tHit) && tHit < bestT){
                    bestT = tHit;
                    bestColor[0] = spheres[s]->c_diff[0];
                    bestColor[1] = spheres[s]->c_diff[1];
                    bestColor[2] = spheres[s]->c_diff[2];
                    hit = true;
                }
            }

            for(int p=0;p<planeCount;p++){
                float tHit;
                if(hit_plane(O, D, planes[p], tHit) && tHit < bestT){
                    bestT = tHit;
                    bestColor[0] = planes[p]->c_diff[0];
                    bestColor[1] = planes[p]->c_diff[1];
                    bestColor[2] = planes[p]->c_diff[2];
                    hit = true;
                }
            }

            int idx = (j*W + i) * 3;
            if(hit){
                pix[idx+0] = to_byte(bestColor[0]);
                pix[idx+1] = to_byte(bestColor[1]);
                pix[idx+2] = to_byte(bestColor[2]);
            } else {
                pix[idx+0] = 0;
                pix[idx+1] = 0;
                pix[idx+2] = 0;
            }
        }
    }

    if(!write_ppm_p3(argv[4], W, H, pix)){
        std::cerr << "Error: Could not write output file " << argv[4] << "\n";
        delete[] pix;
        for(int i=0;i<sphereCount;i++) delete spheres[i];
        for(int i=0;i<planeCount;i++) delete planes[i];
        return 1;
    }

    delete[] pix;
    for(int i=0;i<sphereCount;i++) delete spheres[i];
    for(int i=0;i<planeCount;i++) delete planes[i];

    return 0;
}
