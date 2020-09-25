#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "objdb.h"
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    int database_fd = -1;
    char database_name[MAX_NAME_LEN] = {'\0'};
    char operation = ' ';

    while (scanf(" %c", &operation) != 0 && operation != 'q')
    {
        if (operation == 'o')
        {

            if (database_fd != -1)
                close(database_fd);

            scanf(" %s", database_name);
            database_name[strlen(database_name)] = '\0';

            database_fd = open_database(database_name);

            if (database_fd == -1)
                printf("Error opening %s\n", database_name);
        }
        else if (operation == 'i')
        {
            if (database_fd < 0)
            {
                printf("No open db file\n");
            }
            else
            {
                char file_name[MAX_NAME_LEN] = {'\0'};
                char file_name_in_database[MAX_NAME_LEN] = {'\0'};

                scanf(" %s %s", file_name, file_name_in_database);

                import(database_fd, file_name, file_name_in_database);
            }
        }
        else if (operation == 'f')
        {
            if (database_fd < 0)
            {
                printf("No open db file\n");
            }
            else
            {

                char file_to_search[MAX_NAME_LEN] = {'\0'};

                scanf(" %s", file_to_search);

                off_t_array *f_array = search(database_fd, file_to_search);

                for (int i = 0; i < f_array->size; i++)
                {
                    char file_name[MAX_NAME_LEN] = {'\0'};

                    lseek(database_fd, f_array->location_in_db[i], SEEK_SET);
                    read(database_fd, file_name, f_array->name_size_of_files[i]);
                    printf("%s\n", file_name);
                }

                free(f_array->location_in_db);
                free(f_array->name_size_of_files);
                free(f_array->size_of_files);
                free(f_array);
            }
        }
        else if (operation == 'e')
        {
            if (database_fd < 0)
            {
                printf("No open db file\n");
            }
            else
            {
                char file_name_in_database[MAX_NAME_LEN] = {'\0'};
                char new_file[MAX_NAME_LEN] = {'\0'};

                scanf(" %s %s", file_name_in_database, new_file);

                export(database_fd, file_name_in_database, new_file);
            }
        }
        else if (operation == 'c')
        {
            if (database_fd > -1)
            {
                close(database_fd);
            }
            else
            {
                printf("No open db file");
            }
        }
        else if (operation == 'd')
        {
            char file_name_in_database[MAX_NAME_LEN] = {'\0'};

            scanf(" %s", file_name_in_database);

            if (database_fd > -1)
            {
                delete (database_fd, database_name, file_name_in_database);
            }
            else
            {
                printf("No open db file");
            }
        }
    }

    if (database_fd != -1)
        close(database_fd);
    return 0;
}
