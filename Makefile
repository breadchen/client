
object = client.o responsecode.o#errreport.h config.h

client : $(object)
	gcc -g -o client $(object)
responsecode.o : responsecode.c
	gcc -g -c responsecode.c
client.o : client.c
	gcc -g -c client.c
