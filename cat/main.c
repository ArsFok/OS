#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv){
    if(argc < 2){
        return 1;
    }
    const char* name = argv[1];
    FILE* file = fopen(name, "r");
    if(file == NULL){
        return 1;
    }
    int opt;
    int* flags = (int*)calloc(6, sizeof(int));
    optind = 2;
    while((opt = getopt(argc, argv, "AbeEnstTuv")) != -1){
        switch(opt){
            case 'A':
                flags[1] = 1;
                flags[4] = 1;
                flags[5] = 1;
                break;
            case 'b':
                flags[0] = 1;
                break;
            case 'e':
                flags[1] = 1;
                flags[5] = 1;
                break;
            case 'E':
                flags[1] = 1;
                break;
            case 'n':
                flags[2] = 1;
                break;
            case 's':
                flags[3] = 1;
                break;
            case 't':
                flags[4] = 1;
                flags[5] = 1;
                break;
            case 'T':
                flags[4] = 1;
                break;
            case 'v':
                flags[5] = 1;
                break;
            default:
                break;
        }
    }
   
    for(int i = 0; i < 6; i++){
        printf("%d\t", flags[i]);
    }
    printf("\n");
    fclose(file);
    free(flags);
    return 0;
}