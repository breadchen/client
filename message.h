#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "config.h"
#define MSG_HEAD "CC"
#define MSG_HEAD_LEN 2

struct message
{
	char head[MSG_HEAD_LEN];
	unsigned short len; // cmd length
	char cmd[MAX_MSG_LEN];
};

#endif // if (!_MESSAGE_H)
