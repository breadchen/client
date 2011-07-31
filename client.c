#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "config.h"
#include "errreport.h"

extern int addr_convert(char *addr, struct in_addr *addr_out);
extern int send_command(FILE* fp, int sockfd);

int main(int argc, char** argv)
{
	int n_clientsock = 0;
	struct sockaddr_in serv_addr;

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
	int n_recived;

	while(1)
	{
		if (NULL != fgets(sends, MAX_MSG_LEN, fp))
		{
			if ('\n' == sends[strlen(sends) - 1])
				sends[strlen(sends) - 1] = '\0';
			else // clear stdin
				while('\n' != getchar());
			send(sockfd, sends, strlen(sends) + 1, 0);

			if ('q' == sends[0] || 'Q' == sends[0]) break;

			if (0 == (n_recived = recv(sockfd, recvs, MAX_MSG_LEN, 0)))
			{
				PRINT_ERR("error receiving data")
			} // end of if
			else
			{
				printf("%s\n", recvs);
			} // end of else
		} // end of if
	} // end of while

	return FUC_SUCCESS;
} // end of send_command()
