#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "config.h"
#include "errreport.h"
#include "message.h"
#include "responsecode.h"

extern int addr_convert(char *addr, struct in_addr *addr_out);
extern int send_command(FILE* fp, int sockfd);
extern int send_to_server(int sockfd, char* content, int offset, int len);

int main(int argc, char** argv)
{
	int n_clientsock = 0;
	struct sockaddr_in serv_addr;
	char recvs[MAX_MSG_LEN];

	// init socket
	n_clientsock = socket(AF_INET, SOCK_STREAM, 0);
	if (n_clientsock < 0)
	{
		PRINT_ERR("init socket error")
		PRINT_ERR(strerror(errno))
		exit(EXIT_FAILURE);
	} // end of if

	// init address data	
	if (argc >= LEAST_ARG_NUM &&
		FUC_SUCCESS == addr_convert(argv[1], &serv_addr.sin_addr)
		)
	{
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(SERV_PORT);
	} // end of if
	else
	{ // if can't use arguments, use local address instead
		if (argc >= LEAST_ARG_NUM)
		{
			PRINT_ERR("can not recognize the ip address or hostname")
		} // end of if
		PRINT_ERR("use default server address")

		bzero(&serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(SERV_PORT);
		if (0 == inet_aton(SERV_IP, (struct in_addr*)&serv_addr.sin_addr))
		{
			PRINT_ERR("fail to use default address")
			PRINT_ERR(strerror(errno))
			exit(EXIT_FAILURE);
		} // end of if
	} // end of else

	// connect to server
	if (connect(n_clientsock, 
				(struct sockaddr*)&serv_addr, 
				sizeof(serv_addr)) < 0)
	{
		PRINT_ERR("connect error")
		PRINT_ERR(strerror(errno))
		exit(EXIT_FAILURE);
	} // end of if

	// receive welcome message
	if (FUC_FAILURE != get_response(n_clientsock, recvs))
	{
		printf("server: %s\n", str_response[atoi(recvs)]);

		if (SERVER_BUSY == atoi(recvs))
		{
			exit(EXIT_FAILURE);
		} // end of if

		if (SERVER_QUIT == atoi(recvs))
		{
			exit(EXIT_SUCCESS);
		} // end of if
	} // end of if
	else
	{
		PRINT_ERR("receive welcome message time out")
	} // end of else

	if (FUC_FAILURE == send_command(stdin, n_clientsock))
	{
	} // end of if

	close(n_clientsock);
	return EXIT_SUCCESS;
} // end of main()


/*
 * Function: convert the string of ip address or hostname
 * 	to struct inaddr, success return FUC_SUCCESS,
 * 	failure return FUC_FAILURE
 */
int addr_convert(char *addr, struct in_addr *addr_out)
{
	struct hostent *he;

	if (1 == inet_aton(addr, addr_out))
	{
		return FUC_SUCCESS;
	} // end of if

	he = gethostbyname(addr);
	if (NULL != he)
	{
		*addr_out = *((struct in_addr *)(he->h_addr_list[0]));
		return FUC_SUCCESS;
	} // end of if 

	return FUC_FAILURE;
} // end of addr_convert()


/*
 * Function: get user's input and send it to server
 */
int send_command(FILE* fp, int sockfd)
{
	char sends[MAX_MSG_LEN];
	char recvs[MAX_MSG_LEN];
	int n_received;

	extern int get_response(int sockfd, char* command_out);

	while(1)
	{
		if (NULL != fgets(sends, MAX_MSG_LEN, fp))
		{
			if ('\n' == sends[strlen(sends) - 1])
				sends[strlen(sends) - 1] = '\0';
			else // clear stdin
				while('\n' != getchar());
			
			if (FUC_FAILURE == send_to_server(sockfd, 
											  sends, 
											  0, 
											  strlen(sends) + 1))
			{
				PRINT_ERR("fail to send command")
				//continue;
			} // end of if

			if (4 <= strlen(sends) && 
				(' ' == sends[4] || '\0' == sends[4]) &&
				(0 == strncmp(sends, "quit", 4) || 0 == strncmp(sends, "QUIT", 4))
			   ) break;

			if (FUC_FAILURE == (n_received = get_response(sockfd, recvs)))
			{
				printf("didn't receive response from server, maybe you should try again~\n");
				continue;
			} // end of if
			else
			{ // print response
				printf("server: %s\n", str_response[atoi(recvs)]);

				if (SERVER_QUIT == atoi(recvs))
				{
					exit(EXIT_SUCCESS);
				} // end of if
			} // end of else
		} // end of if
	} // end of while

	return FUC_SUCCESS;
} // end of send_command()


/*
 * Function: wrap command then send it to server
 */
int send_to_server(int sockfd, char* content, int offset, int len)
{
	struct message msg;

	extern int wrapper(struct message*, char*, int, int);
	extern int _send(int sockfd, const struct message* msg);

	if (FUC_FAILURE == wrapper(&msg, content, offset, len))
	{
		return FUC_FAILURE;
	} // end of if
	
	if (FUC_FAILURE == _send(sockfd, &msg))
	{
		return FUC_FAILURE;
	} // end of if

	return FUC_SUCCESS;
} // end of send_to_server()


/*
 * Fuction: send struct message through sock, 
 *  make sure all parts of message have been sent
 */
int _send(int sockfd, const struct message* msg)
{
	int n_counter = 0;
	int n_offset = 0;
	do
	{
		n_counter = send(sockfd, 
						 ((char*)msg) + n_offset,
						 sizeof(struct message) - n_offset,
						 0);

		if (n_counter > 0)
			n_offset += n_counter;
		else
		{
			PRINT_ERR("send error")
			PRINT_ERR(strerror(errno))
			return FUC_FAILURE;
		} // end of else
	} while (n_offset < sizeof(struct message));

	return FUC_SUCCESS;
} // end of _send()


/*
 * Function: put string content in struct message
 */
int wrapper(struct message* msg_out, char* content, int offset, int len)
{
	if (offset < 0 || len < 0 || offset + len > MAX_MSG_LEN) 
		return FUC_FAILURE;

	strncpy(msg_out->head, MSG_HEAD, MSG_HEAD_LEN);
	msg_out->len = len;
	strncpy(msg_out->cmd, &(content[offset]), len);

	return FUC_SUCCESS;
} // end of wrapper()


/*
 * Function: receive response message from server
 *  success return message length, failure return FUC_FAILURE
 */
int get_response(int sockfd, char* command_out)
{
	int n_offset = 0;
	int n_counter = 0;
	char buf[sizeof(struct message)];
	struct message* msg;
	fd_set readfd;
	struct timeval tv;

	// wait few seconds
	tv.tv_sec = TIME_WAIT;
	tv.tv_usec = 0;

	FD_ZERO(&readfd);
	FD_SET(sockfd, &readfd);
	
	do
	{
		select(sockfd + 1, &readfd, NULL, NULL, &tv);
		// time out
		if (!FD_ISSET(sockfd, &readfd)) return FUC_FAILURE;
		
		n_counter = recv(sockfd, 
				         &(buf[n_offset]), 
						 sizeof(struct message) - n_offset, 
						 0);
	
       	if (n_counter > 0)
			n_offset += n_counter; 
		else
		{
			PRINT_ERR("receive error")
			PRINT_ERR(strerror(errno))
			return FUC_FAILURE;
		} // end of if
	} while (n_offset < sizeof(struct message));

	msg = (struct message*)buf;

	strncpy(command_out, msg->cmd, msg->len);

	return msg->len; 
} // end of get_response()
