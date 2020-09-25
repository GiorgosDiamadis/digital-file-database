#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "objdb.h"

int open_database(char const *database_name)
{

    return open(database_name, O_CREAT | O_RDWR, 00600);
}

int export(int database_fd, char file_name_in_database[MAX_NAME_LEN], char new_file[MAX_NAME_LEN])
{
    off_t_array *f_array = search(database_fd, file_name_in_database);

    off_t location_of_file = -1;
    size_t file_size = -1;
    int name_size = -1;

    for (int i = 0; i < f_array->size; i++)
    {
        char file_name[MAX_NAME_LEN] = {'\0'};

        lseek(database_fd, f_array->location_in_db[i], SEEK_SET);
        read(database_fd, file_name, f_array->name_size_of_files[i]);

        if (strcmp(file_name_in_database, file_name) == 0)
        {
            location_of_file = f_array->location_in_db[i];
            name_size = f_array->name_size_of_files[i];
            file_size = f_array->size_of_files[i];
            break;
        }
    }

    free(f_array->location_in_db);
    free(f_array->name_size_of_files);
    free(f_array->size_of_files);
    free(f_array);

    if (location_of_file == -1 || name_size == -1)
    {
        printf("Object %s not found in db\n", file_name_in_database);
        return -1;
    }

    int new_file_fd = open(new_file, O_CREAT | O_EXCL | O_RDWR, 00600);

    if (new_file_fd < 0 && errno == EEXIST)
    {
        printf("Cannot open file %s\n", new_file);
        return -1;
    }

    char *buffer = (char *)calloc(file_size, sizeof(char));

    lseek(database_fd, location_of_file, SEEK_SET);
    lseek(database_fd, name_size + 1, SEEK_CUR);
    lseek(database_fd, sizeof(file_size) + 1, SEEK_CUR);

    read(database_fd, buffer, file_size);

    write(new_file_fd, buffer, file_size);

    close(new_file_fd);

    free(buffer);

    return 1;
}
off_t_array *search(int database_fd, char file_to_search[64])
{
    char *name = (char *)calloc(64, sizeof(char));

    off_t_array *file_descriptors_of_files = (off_t_array *)calloc(1, sizeof(off_t_array));
    file_descriptors_of_files->location_in_db = NULL;
    file_descriptors_of_files->name_size_of_files = NULL;
    file_descriptors_of_files->size_of_files = NULL;

    int number_of_files_found = 0;

    char character = 'a';
    int k = 0;

    for (int i = 0; i < name[i]; i++)
    {
        name[i] = '\0';
    }

    lseek(database_fd, 0, SEEK_SET);

    while (read(database_fd, &character, sizeof(character)) && character != EOF)
    {

        if (character == ' ')
        {

            if (strcmp(file_to_search, "*") == 0)
            {
                add_new_entry(&file_descriptors_of_files, name, database_fd, &number_of_files_found);
            }
            else
            {
                if (strstr(name, file_to_search) != NULL)
                {
                    add_new_entry(&file_descriptors_of_files, name, database_fd, &number_of_files_found);
                }
            }

            //---Clear name string
            for (int i = 0; i < k; i++)
            {
                name[i] = '\0';
            }
            k = 0;
            //--------------------

            //---Move the file descriptor by an offset of file_size+1 in order to check the next file
            size_t file_size = 0;

            read(database_fd, &file_size, sizeof(size_t));

            lseek(database_fd, file_size + 1, SEEK_CUR); // +1 for the empty space " "
            //---------------------------------------------------------------------------------------
        }
        else
        {
            name[k++] = character;
            name[k] = '\0';
        }
    }

    free(name);
    return file_descriptors_of_files;
}
void add_new_entry(off_t_array **f_array, char name[MAX_NAME_LEN], int database_fd, int *number_of_files_found)
{
    off_t location = lseek(database_fd, -strlen(name), SEEK_CUR);

    (*f_array)->location_in_db = (off_t *)realloc((*f_array)->location_in_db,
                                                  (*number_of_files_found + 1) * sizeof(off_t));

    (*f_array)->name_size_of_files = (int *)realloc((*f_array)->name_size_of_files,
                                                    (*number_of_files_found + 1) * sizeof(int));

    (*f_array)->size_of_files = (size_t *)realloc((*f_array)->size_of_files,
                                                  (*number_of_files_found + 1) * sizeof(size_t));

    (*f_array)->location_in_db[*number_of_files_found] = location - 1; //Due to ' '
    (*f_array)->name_size_of_files[*number_of_files_found] = strlen(name);

    lseek(database_fd, strlen(name), SEEK_CUR);

    size_t file_size = 0;

    read(database_fd, &file_size, sizeof(size_t));

    lseek(database_fd, -sizeof(size_t), SEEK_CUR);

    (*f_array)->size_of_files[*number_of_files_found] = file_size;
    (*f_array)->size = ++(*number_of_files_found);
}

int delete (int database_fd, char database_name[MAX_NAME_LEN], char file_to_delete[MAX_NAME_LEN])
{
    off_t_array *all_files = search(database_fd, "*");
    file *files_after_file_to_delete;
    int location = 0;

    char *file_name = (char *)malloc(MAX_NAME_LEN * sizeof(char));
    char *file_content;

    for (int i = 0; i < all_files->size; i++)
    {
        lseek(database_fd, all_files->location_in_db[i], SEEK_SET);
        read(database_fd, file_name, all_files->name_size_of_files[i]);

        file_name[all_files->name_size_of_files[i]] = '\0';

        if (strcmp(file_name, file_to_delete) == 0)
        {
            location = i;
            break;
        }
    }

    if (location == all_files->size - 1)
    {
        close(database_fd);
        size_t new_size = all_files->location_in_db[all_files->size - 1];
        truncate(database_name, new_size);
        open_database(database_name);
        printf("d");
        return 1;
    }

    int num_files = (all_files->size - location - 1);
    files_after_file_to_delete = (file *)malloc(num_files * sizeof(file));
    size_t new_size = all_files->location_in_db[location];

    for (int i = location + 1, k = 0; i < all_files->size; i++, k++)
    {
        for (int j = 0; j < MAX_NAME_LEN; j++)
        {
            file_name[j] = '\0';
        }

        files_after_file_to_delete[k].size = all_files->size_of_files[i];
        files_after_file_to_delete[k].location_in_db = all_files->location_in_db[i];
        files_after_file_to_delete[k].name = (char *)malloc(all_files->name_size_of_files[i] * sizeof(char));
        files_after_file_to_delete[k].content = (char *)malloc(all_files->size_of_files[i] * sizeof(char));

        lseek(database_fd, all_files->location_in_db[i], SEEK_SET);
        read(database_fd, file_name, all_files->name_size_of_files[i]);

        strcpy(files_after_file_to_delete[k].name, file_name);
        lseek(database_fd, sizeof(size_t) + 2, SEEK_CUR);

        file_content = (char *)malloc(all_files->size_of_files[i] * sizeof(char));

        read(database_fd, file_content, all_files->size_of_files[i]);
        strcpy(files_after_file_to_delete[k].content, file_content);
    }

    lseek(database_fd, all_files->location_in_db[location], SEEK_SET);
    int bytes_writen = 0;

    for (int i = 0; i < num_files; i++)
    {
        bytes_writen += write(database_fd, files_after_file_to_delete[i].name, strlen(files_after_file_to_delete[i].name));
        bytes_writen += write(database_fd, " ", 1);
        bytes_writen += write(database_fd, &(files_after_file_to_delete[i].size), sizeof(size_t));
        bytes_writen += write(database_fd, " ", 1);
        bytes_writen += write(database_fd, files_after_file_to_delete[i].content, strlen(files_after_file_to_delete[i].content));
    }
    lseek(database_fd, 0, SEEK_SET);
    close(database_fd);
    printf("%d\n", bytes_writen);

    truncate(database_name, new_size + bytes_writen);
    open_database(database_name);

    return 1;
}

int import(int database_fd, char file_name[MAX_NAME_LEN], char file_name_in_database[MAX_NAME_LEN])
{
    off_t_array *f_array = search(database_fd, file_name_in_database);

    for (int i = 0; i < f_array->size; i++)
    {
        char file_name[MAX_NAME_LEN] = {'\0'};

        lseek(database_fd, f_array->location_in_db[i], SEEK_SET);
        read(database_fd, file_name, f_array->name_size_of_files[i]);
        if (strcmp(file_name_in_database, file_name) == 0)
        {

            free(f_array->location_in_db);
            free(f_array->name_size_of_files);
            free(f_array->size_of_files);
            free(f_array);
            return -1;
        }
    }

    free(f_array->location_in_db);
    free(f_array->name_size_of_files);
    free(f_array->size_of_files);
    free(f_array);

    char *buffer = (char *)malloc(MAX_BYTES * sizeof(char));

    int bytes_read = 0;

    size_t file_size = 0;

    int file_fd = open(file_name, 00400);

    if (file_fd == -1 && errno == ENOENT)
    {
        printf("File does not exist\n");
        return -1;
    }

    //Go to the end of file
    lseek(database_fd, 0, SEEK_END);

    if (file_fd < 0)
    {
        printf("Error opening %s\n", file_name);
    }
    else
    {

        write(database_fd, file_name_in_database, strlen(file_name_in_database) * sizeof(char));

        write(database_fd, " ", sizeof(char));
        bytes_read = read(file_fd, buffer, MAX_BYTES);
        file_size += bytes_read;

        if (bytes_read == MAX_BYTES)
        {
            while (bytes_read == MAX_BYTES)
            {

                buffer = (char *)realloc(buffer, (file_size + MAX_BYTES) * sizeof(char));
                bytes_read = read(file_fd, &buffer[file_size], MAX_BYTES);
                file_size += bytes_read;
            }
        }

        write(database_fd, &file_size, sizeof(size_t));

        write(database_fd, " ", sizeof(char));

        write(database_fd, buffer, file_size);
    }

    free(buffer);

    close(file_fd);
    return 1;
}
