#ifndef __HW3__
#define __HW3__
#include <sys/types.h>

typedef struct file_location_array
{
    off_t *location_in_db;
    size_t *size_of_files;
    int *name_size_of_files;
    int size;
} off_t_array;

typedef struct File
{
    off_t location_in_db;
    size_t size;
    char *name;
    char *content;
} file;

#define MAX_BYTES 512
#define MAX_NAME_LEN 64
extern int open_database(char const *database_name);
extern int import(int database_fd, char file_name[MAX_NAME_LEN], char file_name_in_database[MAX_NAME_LEN]);
extern int export(int database_fd, char file_name_in_database[MAX_NAME_LEN], char new_file[MAX_NAME_LEN]);
extern int delete (int database_fd, char database_name[MAX_NAME_LEN], char file_to_delete[MAX_NAME_LEN]);
extern off_t_array *search(int database_fd, char file_to_search[MAX_NAME_LEN]);
void add_new_entry(off_t_array **f_array, char name[MAX_NAME_LEN], int database_fd, int *number_of_files_found);
#endif