#ifndef _RESPONSE_CODE_H
#define _RESPONSE_CODE_H

#define CMD_COUNT 50
#define RSP_LEN 200 

extern char str_response[CMD_COUNT][RSP_LEN]; 

#define SERVER_BUSY 0
#define WELCOME 1
#define EXEC_SUCCESS 2
#define EXEC_FAILURE 3

#endif
