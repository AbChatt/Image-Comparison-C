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
        char data[3];
        int r_pixel = -1;
        int g_pixel = -1;
        int b_pixel = -1;
        int index = 0;

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
                Pixel *arr = (Pixel *)calloc((img->height * img->width), sizeof(Pixel));
                //arr = {NULL};
                img->p = arr;

                // get pixels
                while (fscanf(img_file, "%d %d %d", &r_pixel, &g_pixel, &b_pixel) != EOF) {
                        if (img->p == 0) {
                                img->p[0].red = r_pixel;
                                img->p[0].green = g_pixel;
                                img->p[0].blue = b_pixel;
                        }
                        else
                        {
                                img->p[index].red = r_pixel;
                                img->p[index].green = g_pixel;
                                img->p[index].blue = b_pixel;
                        }

                        index++;
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
       Pixel new_pixel;
       int row_num = -1;
       int col_num = -1;
       char data[3];
       int eucl_dist = 0;
       int count = 0;

       // process header
       fscanf(img_file, "%s", data);
       fscanf(img_file, "%d %d", &col_num, &row_num);

       // height = # rows
       // width = # cols

       if (col_num == img1->width && row_num == img1->height) {
               for (int i = 0; i < row_num; i++) {
                       for (int j = 0; j < col_num; j++) {
                               fscanf(img_file, "%d %d %d", &new_pixel.red, &new_pixel.green, &new_pixel.blue);
                               eucl_dist = eucl_distance(img1->p[count], new_pixel);
                               count++;
                       }
               }
       }
       else
       {
               return FLT_MAX;
       }
       

       return eucl_dist / count;
}

/* process all files in one directory and find most similar image among them
* - open the directory and find all files in it 
* - for each file read the image in it 
* - compare the image read to the image passed as parameter 
* - keep track of the image that is most similar 
* - write a struct CompRecord with the info for the most similar image to out_fd
*/
CompRecord process_dir(char *dirname, Image *img, int out_fd){

        CompRecord CRec;
        DIR *master_dir = opendir(dirname);
        struct dirent *dp;
        char current_path[PATHLENGTH];
        
        float current_distance = -1.0;
        strncpy(CRec.filename, "", PATHLENGTH);
        CRec.distance = FLT_MAX;

        if (master_dir == NULL) {
                perror("Failed to open directory");
        }
        else
        {
                while ((dp = readdir(master_dir)) != NULL) {
                        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 || strcmp(dp->d_name, ".svn") == 0) {
                                continue;
                        }

                        strncpy(current_path, dirname, PATHLENGTH);
                        strncpy(current_path, "/", PATHLENGTH - strlen(current_path) - 1);
                        strncpy(current_path, dp->d_name, PATHLENGTH - strlen(current_path) - 1);

                        struct stat info;
                        if (stat(current_path, &info) == -1) {
                                perror("Incorrect path or permission denied");
                                exit(1);
                        }

                        if (S_ISREG(info.st_mode)) {
                                current_distance = compare_images(img, current_path);

                                if (current_distance < CRec.distance) {
                                        CRec.distance = current_distance;
                                        strncpy(CRec.filename, current_path, PATHLENGTH);
                                }
                        }
                }
        }
        

        return CRec;
}
