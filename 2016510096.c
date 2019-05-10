#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 128
struct DataInfo
{
    char data_file_name[BUFFER_SIZE];
    char index_file_name[BUFFER_SIZE];
    unsigned keyStart,keyEnd, record_length;
    char encoding[4] , order[5];
};
struct IndexData
{
    char* key;
    unsigned pos;
};
char* pass_whitespaces(char* buffer)
{
    while("\0" != *buffer &&  isspace(*buffer))buffer += 1;
    return buffer;
}
void printMenu()
{
	//print the user menu
	printf("You can perform the following tasks: \n");
	printf("open \"JSON_PATH (including extension name)\"Process JSON file  and load the data and index file if exists. \n");
	printf("create_index   Create index file according the loaded JSON file.\n");
	printf("search \"Key\" search then retrive the according record.\n");
	printf("-------------------------------------\n");
	printf("close Quits\n");
	printf("Please Select one... \n");
    printf("$ ");
}

void freeResources(struct IndexData* indecies , unsigned index_count)
{
    for(size_t i = 0; i < index_count; i++)
    {
        free(indecies[index_count].key);
    }
    free(indecies);
}

int main(int argc, char *argv[])
{
    char answer[BUFFER_SIZE];
    bool should_close = false;
    struct DataInfo data_info;
    struct IndexData* indecies = NULL;
    unsigned index_count = 0;
    while(!should_close)
    {
        //print the user menu and read user answer
        printMenu(answer);
        
	    fgets ( answer , BUFFER_SIZE, stdin);
        fflush(stdin);
        char* buffer_point = pass_whitespaces(answer);
        int lenght_left = BUFFER_SIZE - (buffer_point - answer);
        if(4 < lenght_left && !strncmp(buffer_point, "open", 4))
        {
            buffer_point = pass_whitespaces(buffer_point + 5);
            
            int consumed_int;
            sscanf(buffer_point, "%s%n" , buffer_point , &consumed_int);
            if(0 == consumed_int) continue;
            const char* const file_name = buffer_point;
            if(NULL ==  (buffer_point = strrchr(buffer_point , '.'))) continue;
            if(strncmp(buffer_point, ".json" , 5) && !isspace(buffer_point + 5)) continue;
            *(buffer_point + 5)= '\0'; 
            FILE* json_file = fopen(file_name , "r");
            if(NULL == json_file)
            {
                puts("File is not found!\n");
                continue;
            }
            fseek(json_file, 0, SEEK_END);
            long fsize = ftell(json_file);
            fseek(json_file, 0, SEEK_SET);

            char *json_file_data = malloc(fsize + 1);
            fread(json_file_data, 1, fsize, json_file);
            fclose(json_file);

            json_file_data[fsize] = '\0';

            json_file_data = strchr(json_file_data, '{');


            for(size_t i = 0; i < 7; i++)
            {
                char* start_pos, end_pos;
                json_file_data = pass_whitespaces(json_file_data);
                json_file_data = strchr(json_file_data, '\"') + 1;
                if(!strncmp(json_file_data , "dataFileName" , 12 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    json_file_data = start_pos = pass_whitespaces(json_file_data) + 1;
                    json_file_data = strchr(json_file_data, '\"');
                    memcpy(data_info.data_file_name , start_pos , json_file_data - start_pos);
                    data_info.data_file_name[json_file_data - start_pos] = '\0';
                    json_file_data += 1;
                }
                if(!strncmp(json_file_data , "indexFileName" , 13 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    json_file_data = start_pos = pass_whitespaces(json_file_data) + 1;
                    json_file_data = strchr(json_file_data, '\"');
                    memcpy(data_info.index_file_name , start_pos , json_file_data - start_pos);
                    data_info.index_file_name[json_file_data - start_pos] = '\0';
                    json_file_data += 1;
                }
                if(!strncmp(json_file_data , "keyEncoding" , 11 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    json_file_data = start_pos = pass_whitespaces(json_file_data) + 1;
                    json_file_data = strchr(json_file_data, '\"');
                    memcpy(data_info.encoding , start_pos , json_file_data - start_pos);
                    data_info.encoding[json_file_data - start_pos] = '\0';
                    json_file_data += 1;
                }
                if(!strncmp(json_file_data , "order" , 5 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    json_file_data = start_pos = pass_whitespaces(json_file_data) + 1;
                    json_file_data = strchr(json_file_data, '\"');
                    memcpy(data_info.order , start_pos , json_file_data - start_pos);
                    data_info.order[json_file_data - start_pos] = '\0';
                    json_file_data += 1;
                }
                if(!strncmp(json_file_data , "recordLength" , 12 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    data_info.record_length = atoi(json_file_data);
                }
                if(!strncmp(json_file_data , "keyStart" , 8 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    data_info.keyStart = atoi(json_file_data);
                }
                if(!strncmp(json_file_data , "keyEnd" , 6 ))
                {
                    json_file_data = strchr(json_file_data, ':') + 1;
                    data_info.keyEnd = atoi(json_file_data);
                }
            }
            FILE* index_file = fopen(data_info.index_file_name , "r");
            if(NULL != index_file)
            {
                freeResources(indecies , index_count);
                fseek(index_file, 0, SEEK_END);
                long index_file_size = ftell(index_file);
                fseek(index_file, 0, SEEK_SET);

                index_count = index_file_size / (data_info.keyEnd - data_info.keyStart + sizeof(int)) ; 
                indecies = malloc(index_count * sizeof(struct IndexData));

                for(size_t i = 0; i < index_count; i++)
                {
                    indecies[i].key = malloc(data_info.keyEnd - data_info.keyStart + 1);
                    fread(indecies[i].key , 1 ,  data_info.keyEnd - data_info.keyStart , index_file);
                    indecies[i].key[data_info.keyEnd - data_info.keyStart] = '\0';
                    fread(&indecies[i].pos , sizeof(unsigned) ,  1 , index_file);
                }
                fclose(index_file);
            }
        }
        else if(!strncmp(buffer_point, "create_index", 12))
        {
            freeResources(indecies , index_count);
            FILE* data_file = fopen(data_info.data_file_name , "r");
            fseek(data_file, 0, SEEK_END);
            long data_file_size = ftell(data_file);
            fseek(data_file, 0, SEEK_SET);

            index_count = data_file_size / data_info.record_length ; 
            indecies = malloc(index_count * sizeof(struct IndexData));

            for(size_t i = 0; i < index_count; i++)
            {
                indecies[i].key = malloc(data_info.keyEnd - data_info.keyStart + 1);
                indecies[i].pos = i * data_info.record_length;
                fseek(data_file, indecies[i].pos + data_info.keyStart , SEEK_SET);
                fread(indecies[i].key , 1 ,  data_info.keyEnd - data_info.keyStart , data_file);
                indecies[i].key[data_info.keyEnd - data_info.keyStart] = '\0';
            }
            fclose(data_file);

            int i, j; 
            for (i = 0; i < index_count -1; i++)       
                for (j = 0; j < index_count-i-1; j++)
                {
                    if(!strcmp(data_info.order , "ASC"))
                    {
                        if (strcmp(indecies[j].key ,  indecies[j+1].key) > 0) 
                        {
                            struct IndexData temp;
                            temp.key = indecies[j].key;
                            temp.pos =indecies[j].pos;

                            indecies[j] = indecies[j+1];
                            indecies[j+1] = temp;
                        } 
                    }
                    if(!strcmp(data_info.order , "DESC"))
                    {
                        if (strcmp(indecies[j].key ,  indecies[j+1].key) > 0) 
                        {
                            struct IndexData temp;
                            temp.key = indecies[j + 1].key;
                            temp.pos =indecies[j + 1].pos;

                            indecies[j + 1] = indecies[j];
                            indecies[j] = temp;
                        } 
                    }
                }

            FILE* index_file = fopen(data_info.index_file_name , "w+");
            for(size_t i = 0; i < index_count; i++)
            {
                fwrite(indecies[i].key , 1 , data_info.keyEnd - data_info.keyStart , index_file);
                fwrite(&indecies[i].pos , sizeof(unsigned long) , 1 , index_file);
            }
            fclose(index_file);
        }
        else if(!strncmp(buffer_point, "search", 6))
        {
            buffer_point = pass_whitespaces(buffer_point + 6);
            
            int consumed_int;
            sscanf(buffer_point, "%s%n" , buffer_point , &consumed_int);
            if(0 == consumed_int) continue;
            char* key = buffer_point;

            {
                char* key_end = key;
                while('\0' != *key_end && !isspace(*key_end++));
                *key_end = '\0';
            }

            if(!strcmp(data_info.encoding , "BIN"))
            {
            }
            

            FILE* data_file = fopen(data_info.data_file_name , "r");
            
            int l =0;
            int r = index_count - 1;
            int result_index = -1;
            while (l <= r) 
            { 
                int m = l + (r - l) / 2; 
        
                int result = strcmp(indecies[m].key , key); 
                if(0 == result )
                {
                    result_index = m; 
                    break;
                } 
                else if (0 > result) 
                    l = m + 1; 
        
                else
                    r = m - 1; 
            } 
            if(-1 != result_index )
            {
                fseek(data_file, indecies[result_index].pos, SEEK_SET);
                for(size_t i = 0; i < data_info.record_length; i++)
                    printf("%c" ,fgetc(data_file));
                puts("");
                fclose(data_file);
            }
            else
                puts("Key not found!");
        }
        else if(!strncmp(buffer_point, "close", 5))
        {
            should_close = true;
        }
    }
    freeResources(indecies , index_count);
    return 0;
}