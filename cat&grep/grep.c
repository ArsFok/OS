#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <regex.h>
#include <stdbool.h>

int grep_file(char *pattern, char *filename){
    FILE *file;
    if(filename == NULL || strcmp(filename, "stdin") == 0){
        file = stdin;
    }else{
        file = fopen(filename, "r");
        if(file == NULL){
            fprintf(stderr, "Error opening the file:.%s\n", filename);
            return -1;
        }
    }
    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        if (file != stdin) fclose(file);
        fprintf(stderr, "Invalid regular expression %s\n", pattern);
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    while((read = getline(&line, &len, file)) != -1){
        if (regexec(&regex, line, 0, NULL, 0) == 0) {
            printf("%s", line);
        }
    }
    free(line);
    regfree(&regex);
    if (file != stdin) fclose(file);
    return 0;
}

int main(int argc, char** argv){

    char *filename = NULL;
    char *pattern;
    if(argc < 2){
        fprintf(stderr, "Usage: %s pattern filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    pattern = argv[1];
    if(argc >= 3){
        filename = argv[2];
    }

    int result = grep_file(pattern, filename);
    if (result != 0) {
        exit(EXIT_FAILURE);
    }
    return 0;
}