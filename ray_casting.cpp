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


int main(int argc, char *argv[])
{
    if(argc != 5){
        printf("Usage: raycast width height scene.scene output.ppm\n\n");
        return 1;
    }

    sceneData camera;          // stores camera width/height + magicNum
    sphere* spheres[128];
    plane*  planes[128];
    int sphereCount = 0;
    int planeCount = 0;

	
    if(!readscene(argv[3], &camera, spheres, &sphereCount, planes, &planeCount)){
        return 1;
    }

    return 0;
}

