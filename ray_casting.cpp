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
					scene >> ->radius;
				} else if(property == "color"){
					scene >> scenedata->c_diff[0] >> scenedata->c_diff[1] >> scenedata->c_diff[2];
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

void renderScene(){
	/*for y in the image height
		for x in the image width
			R0 = {0,0,0}
			Rd = {-1/2 camera_width + x * delta_x + offset (1/1000), // delta x camera_width/img_height 
				 -1/2 camera_height + y * delta_y + offset (1/1000), // delta y camera_height/img_height
				 -1}
			normalize(rd);
			
			min_t = infinity;
			intersection_obj = NULL;
			
			for object in object 
				intersection with each object
				object->intersect(R0, Rd) -> t0, t1
				min_t = min(t0, t1) // write function to compare the t values
				if min_t is updated then update intersection obj
				pixmap[...] = intersection, c_diff <- float uint_8 -> x 225
				// if no intersection, assign black*/
}

void write_ppm(){
	// paste code
}
int main(int argc, char *argv[])
{
    if(argc != 5){printf("Usage: raycast width height scene.scene output.ppm\n\n"); return 1;}

    sceneData *scenedata[128];
    readscene(argv[3], *scenedata);


    return 0;
}

