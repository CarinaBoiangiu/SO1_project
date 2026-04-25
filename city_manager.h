#ifndef CITY_MANAGER_H
#define CITY_MANAGER_H

#include <time.h>
#include <sys/types.h>

#define MAX_INSPECTOR 32
#define MAX_CATEGORY 24
#define MAX_DESCRIPTION 128
#define REPORT_SIZE 208 
#define FILE_PATH_SIZE 256

typedef struct {
    int id;                             
    char inspector[MAX_INSPECTOR];      
    float latitude;                     
    float longitude;                    
    char category[MAX_CATEGORY];
    int severity;
    time_t timestamp;
    char description[MAX_DESCRIPTION];  
} Report;


void setup_district(char* district);
int check_permission(const char *filepath, const char *role, int require_write);
void mode_to_string(mode_t mode, char *str);
void log_operation(const char* district, const char* role, const char* user, const char* command);

#endif