#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024

void print_line(const char *line, size_t len, int show_num, unsigned long line_number, int show_eol, int expand_tabs, int numeric, int show_invisibles){
    if(show_num && numeric){
        printf("%6lu\t ", line_number);
    }

    for(size_t i = 0; i < len; i++){
        char ch = line[i];
        
        if(show_eol && ch == '\n'){
            printf("$");
        }

        if(expand_tabs && ch == '\t'){
            printf("\\t");
        }
        else if(show_invisibles && iscntrl((unsigned char)ch) && ch != '\t' && ch != '\n'){
            printf("^%c", ch + 64);
        }
        else{
            putchar(ch);    
        }
    }
}

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "Usage: %s filename [-AbEenstTv]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *filename = NULL;
    int opt;
    int flags[6] = {0};
    //optind = 2;
    while((opt = getopt(argc, argv, "AbeEnstTuv")) != -1){
        switch(opt){
            case 'A': //    -vET
                flags[1] = 1;
                flags[4] = 1;
                flags[5] = 1;
                break;
            case 'b': //num
                flags[0] = 1;
                flags[2] = 1;
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
            case 'u':
                break;
            default:
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                exit(EXIT_FAILURE);
        }
    }
    if (optind < argc) {
        filename = argv[optind];
    } else{
        fprintf(stderr, "No input file specified.\n");
        exit(EXIT_FAILURE);
    }
    FILE *file = fopen(filename, "r");
    if(!file){
        fprintf(stderr, "Error opening the file.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    char buffer[MAX_LINE_LENGTH + 1];

    unsigned long line_number = 1;
    bool last_line_empty = false;

    while(fgets(buffer, sizeof(buffer), file)){
        size_t len = strlen(buffer);
        bool empty_line = (len <= 1 && buffer[len - 1] == '\n');
        if(flags[3] && empty_line && last_line_empty){
            continue;
        }
        last_line_empty = empty_line;
        int should_number = 1;
        if(flags[0] == 1){
            should_number = !empty_line;
        }
        print_line(buffer, len,
                        should_number,
                        line_number,
                        flags[1],
                        flags[4],
                        flags[2],
                        flags[5]);
        line_number++;
    }
    printf("\n");
    fclose(file);
    return 0;
}