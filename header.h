#ifndef OBJECTS_H_
#define OBJECTS_H_

typedef struct{
  	int type; // 0 = camera, 1 = sphere, 2 = light
  	union {
    	struct {
      		double width;
      		double height;
    	} camera;
    	struct {
    		double diffuse_color[3];
    		double specular_color[3];
      		double position[3];
      		double radius;
    	} sphere;
    	struct {
    		double color[3];
      		double position[3];
      		double direction[3];
      		double radial_a2;
      		double radial_a1;
      		double radial_a0;
      		double angular_ao;
    	} light;
  	};
} Object;

#endif // OBJECTS_H_
