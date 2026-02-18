#include "raycast_lib.h"

static void strip_semicolon(std::string &t){
    if(!t.empty() && t.back() == ';') t.pop_back();
}


bool readscene(char file[], sceneData* camera, sphere* spheres[], int* sphereCount, plane* planes[], int* planeCount)
{


    std::ifstream scene(file);

    if(!scene.is_open())
    {
        std::cerr << "Error: Could not open file " << file << std::endl;
        return false;
    }

    camera->cam_width = 0.0f;
    camera->cam_height = 0.0f;
    *sphereCount = 0;
    *planeCount = 0;

    scene >> camera->magicNum;
    if (camera->magicNum != "img410scene")
    {
        std::cerr << "Error: Only img410scene format supported\n";
        return false;
    }

	
        std::string tok;

    while(scene >> tok){
        if(tok == "end") break;


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

        else if(tok == "sphere"){
            if(*sphereCount >= 128){
                std::cerr << "Error: too many spheres\n";
                return false;
            }

            sphere* s = new sphere();
            // defaults
            s->c_diff[0]=s->c_diff[1]=s->c_diff[2]=0.0f;
            s->x=s->y=s->z=0.0f;
            s->radius=0.0f;

            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                strip_semicolon(property);

                if(property == "c_diff:"){
                    scene >> s->c_diff[0] >> s->c_diff[1] >> s->c_diff[2];
                } else if(property == "position:"){
                    scene >> s->x >> s->y >> s->z;
                } else if(property == "radius:"){
                    scene >> s->radius;
                }

                if(endObj) break;
            }

            spheres[*sphereCount] = s;
            (*sphereCount)++;
        }

        else if(tok == "plane"){
            if(*planeCount >= 128){
                std::cerr << "Error: too many planes\n";
                return false;
            }

            plane* p = new plane();
            // defaults
            p->c_diff[0]=p->c_diff[1]=p->c_diff[2]=0.0f;
            p->x=p->y=p->z=0.0f;
            p->normal[0]=p->normal[1]=p->normal[2]=0.0f;

            std::string property;
            while(scene >> property){
                bool endObj = (!property.empty() && property.back() == ';');
                strip_semicolon(property);

                if(property == "c_diff:"){
                    scene >> p->c_diff[0] >> p->c_diff[1] >> p->c_diff[2];
                } else if(property == "position:"){
                    scene >> p->x >> p->y >> p->z;
                } else if(property == "normal:"){
                    scene >> p->normal[0] >> p->normal[1] >> p->normal[2];
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


int main(int argc, char *argv[])
{
    if(argc != 5){
        printf("Usage: raycast width height scene.scene output.ppm\n\n");
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

    // if camera width/height missing, pick defaults
    if(camera.cam_width == 0.0f)  camera.cam_width = 1.0f;
    if(camera.cam_height == 0.0f) camera.cam_height = 1.0f;

    // pixel buffer
    uint8_t* pix = new uint8_t[W * H * 3];

    // camera at origin, view plane at z=-1
    float O[3] = {0.0f, 0.0f, 0.0f};
    float left = -camera.cam_width * 0.5f;
    float top  =  camera.cam_height * 0.5f;

    for(int j=0;j<H;j++){
        for(int i=0;i<W;i++){

            // pixel center -> view plane
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

            // spheres
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

            // planes
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

    // write ppm
    if(!write_ppm_p3(argv[4], W, H, pix)){
        std::cerr << "Error: Could not write output file " << argv[4] << "\n";
        return 1;
    }

    // cleanup
    delete[] pix;
    for(int i=0;i<sphereCount;i++) delete spheres[i];
    for(int i=0;i<planeCount;i++) delete planes[i];

    return 0;
}

