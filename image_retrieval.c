#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <float.h>
#include "worker.h"

int main(int argc, char **argv) {
	
	char ch;
	char path[PATHLENGTH];
	char *startdir = ".";
        char *image_file = NULL;

	while((ch = getopt(argc, argv, "d:")) != -1) {
		switch (ch) {
			case 'd':
			startdir = optarg;
			break;
			default:
			fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
			exit(1);
		}
	}

        if (optind != argc-1) {
	     fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME] FILE_NAME\n");
        } else
             image_file = argv[optind];

	// Open the directory provided by the user (or current working directory)
	
	DIR *dirp;
	if((dirp = opendir(startdir)) == NULL) {
		perror("opendir");
		exit(1);
	} 
	
	/* For each entry in the directory, eliminate . and .., and check
	* to make sure that the entry is a directory, then call run_worker
	* to process the image files contained in the directory.
	*/
		
	struct dirent *dp;
        CompRecord CRec;
		CompRecord temp;

		int fd[2], r;
		pipe(fd);

		CRec.distance = FLT_MAX;
		strncpy(CRec.filename, "", PATHLENGTH);

	while((dp = readdir(dirp)) != NULL) {

		if(strcmp(dp->d_name, ".") == 0 || 
		   strcmp(dp->d_name, "..") == 0 ||
		   strcmp(dp->d_name, ".svn") == 0 ||
		   strcmp(dp->d_name, ".git") == 0) {
			continue;
		}
		strncpy(path, startdir, PATHLENGTH);
		strncat(path, "/", PATHLENGTH - strlen(path) - 1);
		strncat(path, dp->d_name, PATHLENGTH - strlen(path) - 1);

		struct stat sbuf;
		if(stat(path, &sbuf) == -1) {
			//This should only fail if we got the path wrong
			// or we don't have permissions on this entry.
			perror("stat");
			exit(1);
		} 

		// Only call process_dir if it is a directory
		// Otherwise ignore it.
		if(S_ISDIR(sbuf.st_mode)) {
                        printf("Processing all images in directory: %s \n", path);

						// fork child
						r = fork();

						if (r == 0) {
							// child
							close(fd[0]);

							// create Image representation of provided image
							Image *img_file = read_image(image_file);

							// call process_dir on each sub directory
							process_dir(path, img_file, fd[1]);

							// close any open file descriptors and exit
							close(fd[1]);
							exit(0);
						}
						else if (r > 0) {
							// parent

							// no need for while loop as OS will block call till pipe has data
							if (read(fd[0], &temp, sizeof(temp)) == -1) {
								perror("Unable to read from pipe\n");
								exit(1);
							}
							
							if (temp.distance <= CRec.distance) {
								CRec.distance = temp.distance;
								strncpy(CRec.filename, temp.filename, PATHLENGTH);
							}			
						}
						else
						{
							perror("unable to create child process\n");
						}
						
		}
		
	}

        printf("The most similar image is %s with a distance of %f\n", CRec.filename, CRec.distance);
	
	return 0;
}
