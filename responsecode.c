#include "responsecode.h"

char str_response[CMD_COUNT][RSP_LEN] = 
{ 
	// SERVER_BUSY
	"Server too busy, please try again a few minutes later.",
	// WELCOME
	"Welcome!",
	// EXEC_SUCCESS
	"Command executed.",
	// EXEC_FAILURE
	"Command execute error.",
	// SERVER_QUIT
	"Server exit."
};
