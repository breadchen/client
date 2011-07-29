#ifndef _ERR_REPORT_H
#define _ERR_REPORT_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define PRINT_ERR(str) printf("%s\n", (str));
#define PRINT_TEST(str) printf("%s\n", (str));
#define BUFFER_SIZE 1024

char test_buf[BUFFER_SIZE];

#endif // if(!_ERR_REPORT_H)
