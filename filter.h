#ifndef FILTER_H
#define FILTER_H

#include "city_manager.h"

int filter_reports(const char *district, const char *role, const char *user,
                   const char *condition);

// AI generated
int parse_condition(const char *input, char *field, char *op, char *value);
int match_condition(Report *r, const char *field, const char *op,
                    const char *value);

#endif