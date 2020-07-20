/*
Leanne Feng V00825004
open disk file
get disk size
char *mmap = mmap(disk file, ... disk size)

Free size of disk: check FAT table
number of files in root dir: check root directory
other variables: check boot sector

munmap(disk file)
close disk file
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <string.h>



void get_os_name(char *os_name, char *mmap) {	
	int i;
	for(i = 0; i < 8; i++) {
		os_name[i] = mmap[3+i];
	}
}

void get_disk_label(char *disk_label, char *mmap) {
	int base = 9728; // the first byte of the root sector
   	int current = base;
    	int offset = 32; // every entry is 32 bytes
   	int* temp1 = malloc(sizeof(int));
    	int* temp2 = malloc(sizeof(int));
    	int i;
    
    	*temp1 = mmap[current];
    
    	// does not equal to free space and no more files and still inside root directory
    	while (*temp1 != 0x00 && current < (33 * 512)){
        	// check for files
        	if (*temp1 != 0xE5){
            		*temp2 = mmap[current + 11];
            		if (*temp2 == 0x08) {
                		for (i = 0; i < 8; i++){
                   			disk_label[i] = mmap[current + i];
                		}
                		break;
            		}
       		}
        	// move to next entry in the root directory
       		current = current + offset;
        	*temp1 = mmap[current];
    	}
    
    	free(temp1);
	free(temp2);
	
}

int get_total_size(char* mmap){
	int* temp1 = malloc(sizeof(int));
	int* temp2 = malloc(sizeof(int));
	int value;
    	// mmap to the 19th and 20th byte
    	*temp1 = mmap[19];
    	*temp2 = mmap[20];
    
    	// switch to Little Endian
    	value = *temp1 + ((*temp2) << 8);
    
    	free(temp1);
   	free(temp2);
    
    	return value;

}


int get_total_fats (char* mmap){
	int return_value = mmap[16]; // map to byte 16
	return return_value;
}

int sectors (char* mmap){
	int* temp1 = malloc(sizeof(int));
	int* temp2 = malloc(sizeof(int));
	int value;
    
	// mmap to the 22nd and 23rd byte
	*temp1 = mmap[22];
	*temp2 = mmap[23];
    
    	// switch to Little Endian
   	value = *temp1 + ((*temp2) << 8);
    
    	free(temp1);
    	free(temp2);
    
    	return value;
}


int get_free_size(char* mmap, int numSectors){
	int n = 2; // logical number of the first sector in the Data Area
	int base = 512; // the first bye of the FAT table
	int* temp1 = malloc(sizeof(int));
	int* temp2 = malloc(sizeof(int));
    	int result = 0;
    	int counter = 0;
   
   	 // The logical number for all the sectors in the Data Area is from 2 to 2484
    	for (n = 2; n <= (numSectors - 32); n++){
        	// if the logical number is even
        	if (n % 2 == 0){
            		// get all 8 bits
            		*temp1 = mmap[base + 3 * n/2];
            		*temp2 = mmap[base + 1 + 3 * n/2];
            		*temp2 = *temp2 & 0x0F; // get the low 4 bits
            
            		// apply "Little Endian"
            		result = ((*temp2) << 8) + *temp1;
       
        	} else {
            		// get all 8 bits
            		*temp1 = mmap[base + 3 * n/2];
            		*temp2 = mmap[base + 1 + 3 * n/2];
            		*temp1 = *temp1 & 0xF0; // get the low 4 bits

            		// apply "Little Endian"
            		result = ((*temp1) << 8) + *temp2;
        	}
        
        	// check if the value is 0x00 --> sector is free/unused
        	if (result == 0x00) {
            		counter++;
        	}
    	}
    
    	free(temp1);
    	free(temp2);
    
    	return counter;
}

int get_total_files_in_root(char* mmap){
	int base = 9728;  // the first byte of the root directory
	int current = base;   // point to the first byte of the current entry
	int offset = 32;  // Each entry has 32 bytes in root directory
	int* temp1 = malloc(sizeof(int));
	int* temp2 = malloc(sizeof(int));
	int counter = 0;
   
   	*temp1 = mmap[current];
    
    	while(*temp1 != 0x00 && current < (33 * 512)){
        	// 0xE5 -> empty
        	if (*temp1 != 0xE5){
            		*temp2 = mmap[current + 11];
            		// check to see if not part of a: long filename: 0x0F, subdirectory: 0x10 and volume label: 0x08
            		// then it is a file
           		if (*temp2 != 0x0F && *temp2 != 0x08 && *temp2 != 0x10){
               	 		counter ++;
            		}
        	}
        
        	// Go to next entry in Root Directory
        	current = current + offset;
        	*temp1 = mmap[current];
    	}
    	free(temp1);
    	free(temp2);
    
    	return counter;
}

int main(int argc, char *argv[]) {
	if(argc != 2){
		printf("Inappropriate command line format.\n");
		return -1;
	}

	char *fd_image = argv[1];
	char *os_name = malloc(sizeof(char) * 8);
	char *disk_label = malloc(sizeof(char) * 11);
	int disk_size_total = 0;
	int disk_size_free = 0;
	int numFiles_in_root = 0;
	int numFat_copies = 0;

	int fd;
	struct stat file_stats;
	char *map;

	if((fd = open(fd_image, O_RDONLY))) {
		// Return information about the file and store it in file_stats
		fstat(fd, &file_stats);
		//char *mmap = mmap(disk file, ... disk size)
		map = mmap(NULL, file_stats.st_size, PROT_READ, MAP_SHARED, fd, 0);
		
		get_os_name(os_name, map);
		get_disk_label(disk_label, map);
		disk_size_total = get_total_size(map);
		disk_size_free = get_free_size(map,disk_size_total);	
		numFiles_in_root = get_total_files_in_root(map);
		numFat_copies = get_total_fats(map);
		int num_sectors = sectors(map);
	
		printf("OS Name: %s\n", os_name);
		printf("Label of the disk: %s\n", disk_label);
		printf("Total size of the disk: %d\n", disk_size_total* 512);
		printf("Free size on the disk: %d\n", disk_size_free* 512);	
		printf("================================================\n");
		printf("The number of files in the root directory (not including subdirectories): %d\n", numFiles_in_root);
		printf("================================================\n");
		printf("Number of FAT copies: %d\n", numFat_copies);
		printf("Sectors per FAT: %d\n", num_sectors);
	} else {
		perror("Error opening file for reading");
		exit(EXIT_FAILURE);
	}

	free(os_name);
	close(fd);		//close disk file
	return 0;
}
