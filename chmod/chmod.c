#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

void change_permissions(mode_t mode){

}

int main(int argc, char** argv){
    if(argc != 3){
        fprintf(stderr, "");

    }
    const char *mode_str = argv[1];
    const char *filename = argv[2];

    if(!file_exist(filename)){
        fprintf(stderr, "Error: %s file does not exist");
        exit(EXIT_FAILURE);
    }
    return 0;
}