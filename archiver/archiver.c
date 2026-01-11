#include "archive.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_help();
        return 1;
    }
    
    // Вывод справки
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }
    
    if (argc < 3) {
        print_help();
        return 1;
    }
    
    const char *archive_name = argv[1];
    const char *option = argv[2];
    
    // Добавление файла в архив
    if ((strcmp(option, "-i") == 0 || strcmp(option, "--input") == 0) && argc >= 4) {
        return add_to_archive(archive_name, argv[3]);
    }
    // Извлечение файла из архива
    else if ((strcmp(option, "-e") == 0 || strcmp(option, "--extract") == 0) && argc >= 4) {
        return extract_from_archive(archive_name, argv[3]);
    }
    // Показать содержимое архива
    else if (strcmp(option, "-s") == 0 || strcmp(option, "--stat") == 0) {
        return show_archive_info(archive_name);
    }
    else {
        print_help();
        return 1;
    }
}