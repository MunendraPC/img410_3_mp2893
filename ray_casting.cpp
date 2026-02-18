#include "raycast_lib.h"

bool readscene(char file[], sceneData *scenedata){
    std::ifstream scene(file);

    if(!scene.is_open())
    {
        std::cerr << "Error: Could not open file " << file << std::endl;
        return false;
    }

    scene >> scenedata->magicNum;
    if (scenedata->magicNum != "img410")
	{
        std::cerr << "Error: Only img410 format supported\n";
        return false;
	}

    pixmap = new uint8_t[metadata->width * metadata.height * 3];
	
    std::string end;
    std::string object;
    int val = 0;

	while(scene >> end && end != "end"){

        // if the object is a sphere
		if(end == "sphere"){
			std::string property;
			while(scene >> property && property != "sphere" && property != "camera" && property != "plane" && property != "end"){
				if(property == "position"){
					scene >> scenedata->x >> scenedata->y >> scenedata->z;
				} else if(property == "radius"){
					float r;
					scene >> radius;
				} else if(property == "color"){
					int r, g, b;
					scene >> r >> g >> b;
				}
			}
			end = property;
			if(end == "end") break;
			continue;

        // if the object is a camera
		} else if(end == "camera"){
			std::string property;
			while(scene >> property && property != "sphere" && property != "camera" && property != "plane" && property != "end"){
				if(property == "position"){
					float x, y, z;
					scene >> x >> y >> z;
				} else if(property == "direction"){
					float x, y, z;
					scene >> x >> y >> z;
				}
			}
			end = property;
			if(end == "end") break;
			continue;
        // if the object is a plane
		} else if(end == "plane"){
			std::string property;
			while(scene >> property && property != "sphere" && property != "camera" && property != "plane" && property != "end"){
				if(property == "position"){
					float x, y, z;
					scene >> x >> y >> z;
				} else if(property == "normal"){
					float x, y, z;
					scene >> x >> y >> z;
				} else if(property == "color"){
					int r, g, b;
					scene >> r >> g >> b;
				}
			}
			end = property;
			if(end == "end") break;
			continue;
           

        }
	}
} // hold in an array of ptrs, linked list, or static array (objects[])

int main(int argc, char *argv[])
{
    if(argc != 5){printf("Usage: raycast width height scene.scene output.ppm\n\n"); return 1;}

    sceneData *scenedata[128];
    readscene(argv[3], *scenedata);


    return 0;
}

