#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <ctype.h>

#define MAX_MODES 10

struct file_mode {
    char role;
    int permissions[3];
    char modifier;
};

int is_role_char(char ch) {
    return ch == 'a' || ch == 'u' || ch == 'g' || ch == 'o';
}

int is_permission_char(char ch) {
    return ch == 'r' || ch == 'w' || ch == 'x';
}

int is_modifier_char(char ch) {
    return ch == '+' || ch == '-' || ch == '=';
}

char **split_string(const char *str, const char *delim, int *count) {
    if (!str || !delim) return NULL;
    
    char *temp = strdup(str);
    if (!temp) return NULL;
    
    char *token = strtok(temp, delim);
    char **result = NULL;
    int capacity = 0;
    *count = 0;
    
    while (token) {
        if (*count >= capacity) {
            capacity = capacity ? capacity * 2 : 4;
            char **new_result = realloc(result, capacity * sizeof(char *));
            if (!new_result) {
                free(temp);
                free(result);
                return NULL;
            }
            result = new_result;
        }
        
        result[*count] = strdup(token);
        if (!result[*count]) {
            free(temp);
            for (int i = 0; i < *count; i++) free(result[i]);
            free(result);
            return NULL;
        }
        (*count)++;
        token = strtok(NULL, delim);
    }
    
    free(temp);
    return result;
}

void free_string_array(char **arr, int count) {
    if (!arr) return;
    for (int i = 0; i < count; i++) free(arr[i]);
    free(arr);
}

int parse_mode_pattern(const char *pattern, struct file_mode *fm) {
    if (!pattern || !fm) return -1;
    
    int i = 0;
    int has_role = 0;
    
    while (pattern[i] && is_role_char(pattern[i])) {
        if (!has_role) {
            fm->role = pattern[i];
            has_role = 1;
        } else if (fm->role != pattern[i]) {
            fm->role = 'a';
        }
        i++;
    }
    if (!has_role) {
        fm->role = 'a';
    }
    
    if (!pattern[i] || !is_modifier_char(pattern[i])) {
        return -1;
    }
    
    fm->modifier = pattern[i];
    i++;
    for (int j = 0; j < 3; j++) {
        fm->permissions[j] = 0;
    }
    
    int has_permissions = 0;
    while (pattern[i] && is_permission_char(pattern[i])) {
        has_permissions = 1;
        switch (pattern[i]) {
            case 'r': fm->permissions[0] = 1; break;
            case 'w': fm->permissions[1] = 1; break;
            case 'x': fm->permissions[2] = 1; break;
        }
        i++;
    }
    
    if (fm->modifier == '=' && !has_permissions) {
        for (int j = 0; j < 3; j++) {
            fm->permissions[j] = 0;
        }
    }
    
    return (pattern[i] == '\0') ? 0 : -1;
}

void apply_permissions(mode_t *mode, const struct file_mode *fm) {
    if (!mode || !fm) return;
    
    mode_t masks[3][3] = {
        { S_IRUSR, S_IWUSR, S_IXUSR },
        { S_IRGRP, S_IWGRP, S_IXGRP },
        { S_IROTH, S_IWOTH, S_IXOTH } 
    };
    
    int role_start, role_end;
    
    switch (fm->role) {
        case 'u': role_start = 0; role_end = 0; break;
        case 'g': role_start = 1; role_end = 1; break;
        case 'o': role_start = 2; role_end = 2; break;
        case 'a': role_start = 0; role_end = 2; break;
        default: return;
    }
    
    for (int role = role_start; role <= role_end; role++) {
        for (int perm = 0; perm < 3; perm++) {
            if (fm->permissions[perm]) {
                switch (fm->modifier) {
                    case '+':
                        *mode |= masks[role][perm];
                        break;
                    case '-':
                        *mode &= ~masks[role][perm];
                        break;
                    case '=':
                        if (fm->permissions[perm]) {
                            *mode |= masks[role][perm];
                        } else {
                            *mode &= ~masks[role][perm];
                        }
                        break;
                }
            } else if (fm->modifier == '=') {
                *mode &= ~masks[role][perm];
            }
        }
    }
}

mode_t get_current_mode(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    }
    return S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
}

mode_t parse_symbolic_mode(const char *mode_str, const char *filename) {
    int pattern_count;
    char **patterns = split_string(mode_str, ",", &pattern_count);
    if (!patterns || pattern_count == 0) {
        fprintf(stderr, "Failed to parse mode patterns\n");
        exit(EXIT_FAILURE);
    }
    
    struct file_mode file_modes[MAX_MODES];
    int valid_modes = 0;
    
    for (int i = 0; i < pattern_count && valid_modes < MAX_MODES; i++) {
        if (parse_mode_pattern(patterns[i], &file_modes[valid_modes]) == 0) {
            valid_modes++;
        } else {
            fprintf(stderr, "Invalid mode pattern: %s\n", patterns[i]);
        }
    }
    
    mode_t result_mode;
    if (filename) {
        result_mode = get_current_mode(filename);
    } else {
        result_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    }
    
    for (int i = 0; i < valid_modes; i++) {
        apply_permissions(&result_mode, &file_modes[i]);
    }
    
    free_string_array(patterns, pattern_count);
    return result_mode;
}
mode_t parse_octal_mode(const char *mode_str) {
    if (strlen(mode_str) > 4) {
        fprintf(stderr, "Mode must have maximum 4 octal digits\n");
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; mode_str[i] != '\0'; i++) {
        if (mode_str[i] < '0' || mode_str[i] > '7') {
            fprintf(stderr, "Invalid octal digit: %c\n", mode_str[i]);
            exit(EXIT_FAILURE);
        }
    }
    
    unsigned int omode;
    sscanf(mode_str, "%o", &omode);
    return omode & (S_IRWXU | S_IRWXG | S_IRWXO);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <mode> <file1> [file2 ...]\n", argv[0]);
        fprintf(stderr, "Mode can be:\n");
        fprintf(stderr, "  Octal: 755, 644, etc.\n");
        fprintf(stderr, "  Symbolic: u+x, go-w, a=rw, u+rwx,g+rx,o+r, etc.\n");
        exit(EXIT_FAILURE);
    }
    
    int success_count = 0;
    
    if (isdigit(argv[1][0])) {
        mode_t new_mode = parse_octal_mode(argv[1]);
        
        for (int i = 2; i < argc; i++) {
            if (chmod(argv[i], new_mode) == 0) {
                success_count++;
            } else {
                fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
            }
        }
    } 
    else {
        for (int i = 2; i < argc; i++) {
            mode_t new_mode = parse_symbolic_mode(argv[1], argv[i]);
            if (chmod(argv[i], new_mode) == 0) {
                success_count++;
            } else {
                fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
            }
        }
    }
    
    if (success_count > 0) {
        printf("Successfully changed permissions for %d files\n", success_count);
    }
    
    return (success_count == argc - 2) ? 0 : 1;
}