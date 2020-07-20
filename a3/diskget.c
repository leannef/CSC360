/*

// diskget

open disk file
get disk size
char *src = mmap(disk file, ... disk size)

check for file to be copied in disk root dir, grab its file size & related info
open a file in current directory with same size
char *dest = mmap(new file, ... file size)
copy file from src->dest, reading sector by sector

munmap(disk file)
munmap(file)

close disk file
close file
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <string.h>
#include <errno.h>

typedef struct diskFile {
	char* name;
	char* extension;
	int filesize;
	int logical_sector;
}diskFile;


int get_sector (char* mmap, int currSector){
	int* temp1 = malloc(sizeof(int));
	int* temp2 = malloc(sizeof(int));
	int result = 0;
    
    	// if the logical number is even
    	if (currSector % 2 == 0){
        	*temp1 = (mmap[512 + 3 * currSector/2] & 0x00FF);
        	*temp2 = (mmap[512 + 1 + 3 * currSector/2] & 0x00FF);
		// get the low 4 bits        	
		*temp2 = *temp2 & 0x0F;    
        	//Switch to Little Endian format
       	 	result = ((*temp2) << 8) + *temp1;
        
    	} else{
        	*temp1 = (mmap[512 + 3 * currSector/2] & 0x00FF);
        	*temp2 = (mmap[512 + 1 + 3 * currSector/2] & 0x00FF);
       	 	*temp1 = ((*temp1 & 0xF0) >> 4); 
        	result = ((*temp2) << 4) + *temp1;
    	}  
    	free(temp1);
   	free(temp2);
	return result;
}

void get_fileData (char* mmap, char* filename){
	int base = 9728;  // The first byte of the root directory
	int current = base;   // Point to the first byte of the current entry
    	int offset = 32;  // Each entry has 32 bytes in root directory

    	int* temp1 = malloc(sizeof(int));
    	int* temp2 = malloc(sizeof(int));
    	char* temp3 = malloc(sizeof(char)*2);
    	char* check_name = malloc(sizeof(char)*14);
    	diskFile* file = calloc(1, sizeof(diskFile));
    
   	*temp1 = mmap[current];
    
    	while(*temp1 != 0x00 && current < (33 * 512)){
        	char* whole_file_name = calloc(14,sizeof(char));
        	// Check if temp1 is not empty --> 0xE5
        	if (*temp1 != 0xE5){
            		*temp2 = mmap[current + 11];
           	// Check if temp2 is not a volume label --> 0x08
            		if (*temp2 != 0x08){
               			// Check if temp2 is not part of a long file name --> 0x0F
                		if (*temp2 != 0x0F){
                			file -> name = calloc(1, sizeof(char)*9); // Times 9 because 8 bytes + null
                    			file -> extension = calloc(1, sizeof(char)*4); // Times 4 because 3 bytes + null
                    
                    			int i = 0;
                    			for (; i < 8; i++){
                        			file -> name[i] = mmap[current + i];
                        			// If the file name is a space then change to null termination character
                        			if (file -> name[i] == ' '){
                            				file -> name[i] = '\0';
                            				break;
                        			}//end if
                    			}//end for
                    
                    			for (; i < 11; i++){
                        			file -> extension[i - 8] = mmap[current + i];
                    			}
                    
                   			// Null termination
                    			file -> name[8] = '\0';
                    			file -> extension[3] = '\0';
                    			//found the name, type and extension of the file
                   			strcat(whole_file_name, file -> name);
                    			strcat(whole_file_name, ".");
                    			strcat(whole_file_name, file -> extension);
                    	
                   			if(!strcmp(whole_file_name, filename)) {
                        			// Find the logical sector
                        			file -> logical_sector = (mmap[current + 26] & 0x00FF) + ((mmap[current + 27] & 0x00FF) << 8);
                        			file -> filesize = (mmap[current + 28] & 0x00FF) + ((mmap[current + 29] & 0x00FF) << 8) + ((mmap[current + 30] & 0x00FF) << 16) + ((mmap[current + 31] & 0x00FF) << 24);
                        			strcpy(check_name, whole_file_name);
                        			break; // Found the file, break out from the while loop
                    			}
                		}
            		}
        	}
        
        // Go to next entry in Root Directory
	current = current + offset;
        *temp1 = mmap[current];
        free(whole_file_name);
	}//end while
    
	if(strcmp(check_name, filename)){
        	printf("File not found.\n");
        	exit(0);
    	}
    
   	FILE* fp = fopen(filename, "w");
    	int next = file -> logical_sector; 
    	int bytes_left = file -> filesize; 
    	int bytes_to_read;
    
   	for (;next < 0xFF8; next = (get_sector(mmap,next) & 0xFFF), bytes_left -= 512){
        	int start_byte = 512 * (next + 31);
        	if (bytes_left >= 512){
            		bytes_to_read = 512;
        	} else {
           		bytes_to_read = bytes_left;
       		}
        	fwrite (mmap + start_byte, sizeof(char), bytes_to_read, fp);
   	 }
    
    	free(file);
   	free(temp1);
   	free(temp2);
    	free(temp3);
    	free(check_name);
    	fclose(fp);
}

int main (int argc, char** argv){
	int fd;
	struct stat file_stats;
    	char* map;

	if(argc != 3){
		printf("Inappropriate command line format.\n");
		return -1;
	}

    	if ((fd = open (argv[1], O_RDONLY))){	//open disk file
        	fstat(fd, &file_stats);
        	map = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED, fd, 0);       
        	get_fileData(map, argv[2]);	//call method get_fileDta to get information
   	} else{
        	printf ("Fail to open the image file.\n");
    	}

    	close(fd);
    	return 0;
}
