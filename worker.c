#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <math.h>
#include <float.h>
#include "worker.h"


/*
 * Read an image from a file and create a corresponding 
 * image struct 
 */

Image* read_image(char *filename)
{
        Image *img = NULL;
        FILE *img_file = fopen(filename, "r");
        Pixel *root = NULL;
        char data[3];
        int r_pixel = -1;
        int g_pixel = -1;
        int b_pixel = -1;

        img->p = NULL;

        // height = # rows
        // width = # cols

        fscanf(img_file, "%s", data);
        printf("%s\n", data);

        if (strcmp(data, "P3") == 0) {
                // get header
                fscanf(img_file, "%d %d", &img->width, &img->height);
                printf("%d\n", img->width);
                fscanf(img_file, "%d", &img->max_value);
                printf("%d\n", img->height);

                // make space for pixels
                Pixel *arr = malloc((img->height * img->width) * sizeof(Pixel));
                arr = {NULL};
                img->p = arr;

                // get pixels
                while (fscanf(img_file, "%d %d %d", &r_pixel, &g_pixel, &b_pixel) != EOF) {
                        root = img->p;
                        while (root != NULL) {
                                root = *(root + 1);
                        }
                        root->red = r_pixel;
                        root->green = g_pixel;
                        root->blue = b_pixel;
                }

        }
        else
        {
                printf("Error: Incorrect/missing magic number. Header should start with P3\n");
        }
        
        return img;
}

/*
 * Print an image based on the provided Image struct 
 */

void print_image(Image *img){
        printf("P3\n");
        printf("%d %d\n", img->width, img->height);
        printf("%d\n", img->max_value);
       
        for(int i=0; i<img->width*img->height; i++)
           printf("%d %d %d  ", img->p[i].red, img->p[i].green, img->p[i].blue);
        printf("\n");
}

/*
 * Compute the Euclidian distance between two pixels 
 */
float eucl_distance (Pixel p1, Pixel p2) {
        return sqrt( pow(p1.red - p2.red,2 ) + pow( p1.blue - p2.blue, 2) + pow(p1.green - p2.green, 2));
}

/*
 * Compute the average Euclidian distance between the pixels 
 * in the image provided by img1 and the image contained in
 * the file filename
 */

float compare_images(Image *img1, char *filename) {
       FILE *img_file = fopen(filename, "r");      
       return 0;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it 
* - for each file read the image in it 
* - compare the image read to the image passed as parameter 
* - keep track of the image that is most similar 
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
// CompRecord process_dir(char *dirname, Image *img, int out_fd){

//         CompRecord CRec;

//         return CRec;
// }
