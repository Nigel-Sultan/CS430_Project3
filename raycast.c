#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.c"

// Polymorphism in C

typedef struct Pixel{
    unsigned int r;    //create a pixel struct like discussed in class
    unsigned int g;
    unsigned int b;
}Pixel;

typedef struct Image{   //image struct that keeps track of the data in the input file
    int width;
    int height;
    int color;
    unsigned char *data;
} Image;

Image* RayCasting(char*, int, int, Object**);
int writeImage(char*, char*, Image*);

int main(int argc, char **argv) {
    if(argc > 5){
        fprintf(stderr, "Error: Too many arguments!\n");
        return(1);
    }

    char *width = argv[1];
    char *height = argv[2];
    char *inputFilename = argv[3];
    char *outputFilename = argv[4];

    Object** objects = malloc(sizeof(Object*) * 128);

    int w = atoi(width);
    int h = atoi(height);
    if(w<=0){
        fprintf(stderr, "Error: width has to be a positive number!\n");
        return 1;
    }
    if(h<=0){
        fprintf(stderr, "Error: height has to be a positive number!\n");
        return 1;
    }

    read_scene(inputFilename, objects);
    Image* buffer = RayCasting(inputFilename, w, h, objects);

    buffer->width = w;
    buffer->height = h;
    writeImage("P3", outputFilename, buffer);
  return 0;
}

static inline double sqr(double v) {
  return v*v;
}


static inline void normalize(double* v) {
  double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= len;
  v[1] /= len;
  v[2] /= len;
}


double sphere_intersection(double* Ro, double* Rd, double* Center, double r){
   double a = sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]);
  	double b = 2*(Ro[0]*Rd[0] + Ro[1]*Rd[1] + Ro[2]*Rd[2] - Rd[0]*Center[0] - Rd[1]*Center[1] - Rd[2]*Center[2]);
  	double c = sqr(Ro[0]) + sqr(Ro[1]) + sqr(Ro[2]) + sqr(Center[0]) +
             sqr(Center[1]) + sqr(Center[2]) - 2*(Ro[0]*Center[0]
+ Ro[1]*Center[1] + Ro[2]*Center[2]) - sqr(r);
    double d = sqr(b) - 4*a*c;
    if(d < 0){
 	return -1;
    }
    d = sqrt(d);
    double t1 = (-b - d) / (2 * a);
    double t2 = (-b + d) / (2 * a);
    if(t1 > 0){
        return t1;
    }
    if(t2 > 0){
        return t2;
    }
    return -1;
}

double planeIntersection(double* Ro, double* Rd, double* position, double* normal){
    double t = - (normal[0]*Ro[0] + normal[1]*Ro[1] + normal[2]*Ro[2] - normal[0]*position[0]
                - normal[1]*position[1] - normal[2]*position[2]) / (normal[0]*Rd[0]
                + normal[1]*Rd[1] + normal[2]*Rd[2]);

    if (t > 0){
        return t;
    }
    return -1;
}

int writeData(char ppmnum, FILE *outputFile, Image* buffer){
    if(ppmnum == '6'){
        fwrite(buffer->data, sizeof(Pixel), buffer->width*buffer->height, outputFile);
        printf("File saved successfully!\n");
        return(0);
    }
    else if(ppmnum == '3'){
        int i, j;
        for(i=0; i<buffer->height; i++){
            for(j=0; j<buffer->width; j++){
                fprintf(outputFile, "%d %d %d ", buffer->data[i*buffer->width*3+j*3], buffer->data[i*buffer->width*3+j*3+1], buffer->data[i*buffer->width*3+2]);
            }
            fprintf(outputFile, "\n");
        }
        printf("The file saved successfully!\n");
        return(0);
    }
    else{
        fprintf(stderr, "Error: incorrect ppm version\n");
        return(1);
    }
}

int writeImage(char* outnumber, char* outputFilename, Image* buffer){
    int width = buffer->width;
    int height = buffer->height;
    char ppmNum = outnumber[1];
    FILE *fh = fopen(outputFilename, "wb");
    if(fh == NULL){
        fprintf(stderr, "Error: cannot open the file\n");
        return(1);
    }

    char *comment = "# output.ppm";

    fprintf(fh, "P%c\n%s\n%d %d\n%d\n", ppmNum, comment, width, height, 255);
    writeData(ppmNum, fh, buffer);
    fclose(fh);
    return(0);
}

int intersect(double* Rd, int objectNum, Object** objects){
    int closestObjectNum = -1;
    double closest_t = INFINITY;
    int i;
    double t;
    double Ro[3] = {0, 0, 0};
    for(i=0; i<objectNum; i++){
        if(objects[i]->type==1){
            t=sphere_intersection(Ro, Rd, objects[i]->sphere.position, objects[i]->sphere.radius);
            if(t){
                if(t>0 && t<=closest_t){
                    closest_t=t;
                    closestObjectNum=i;
                }
            }
            else{
                fprintf(stderr, "Error: cannot find distance\n");
                return(1);
            }
        }
        else if(objects[i]->type == 2){
            t=planeIntersection(Ro, Rd, objects[i]->plane.position, objects[i]->plane.normal);
            if(t){
                if(t>0 && t<=closest_t){
                    closest_t=t;
                    closestObjectNum=i;
                }
            }
            else{
                fprintf(stderr, "Error: cannot find distance\n");
                return (1);
            }
        }
    }
    return closestObjectNum;
}

Image* RayCasting(char* filename, int w, int h, Object** objects){
    Image* buffer = (Image*)malloc(sizeof(Image));
    if(objects[0] == NULL){
        fprintf(stderr, "Error: no object found");
        exit(1);
    }

    int cameraFound = 0;
    double width;
    double height;
    int i;
    for(i=0; objects[i] != 0; i+=1){
        if(objects[i]->type==0){
            cameraFound=1;
            width=objects[i]->camera.width;
            height=objects[i]->camera.height;
            if(width<=0 || height<=0){
                fprintf(stderr, "Error: invalid size for camera/n");
                exit(1);
            }
        }
    }

    if(cameraFound==0){
        fprintf(stderr, "Error: Camera is not found\n");
        exit(1);
    }

    buffer->data = (unsigned char*)malloc(w*h*sizeof(Pixel));
    Pixel *pixel = (Pixel*)malloc(sizeof(Pixel));
    if(buffer->data==NULL || buffer==NULL){
        fprintf(stderr, "Error: cannot allocate memory\n");
        exit(1);
    }

    double pixwidth = width/w;
    double pixheight = height / h;
    int j, k;

    for(k=0; k<h; k++){
        int count = (h-k-1)*w*3;
        double vy = -height/2 + pixheight * (k+0.5);
        for(j=0; j<h; j++){
            double vx = -width/2+pixwidth*(j+0.5);
            double Rd[3] = {vx, vy, 1};

            normalize(Rd);
            int intersection = intersect(Rd, i, objects);
            if(intersection>=0){
                pixel->r = (int)((objects[intersection]->color[0])*255);
                pixel->g = (int)((objects[intersection]->color[1])*255);
                pixel->b = (int)((objects[intersection]->color[2])*255);
            }
            else{
                pixel->r = 0;
                pixel->g = 0;
                pixel->b = 0;
            }
            buffer->data[count++] = pixel->r;
            buffer->data[count++] = pixel->g;
            buffer->data[count++] = pixel->b;
        }
    }
    return buffer;
}
