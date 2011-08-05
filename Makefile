
object = client.o responsecode.o
configs = errreport.h config.h

client : $(object)
	gcc -g -o client $(object)
responsecode.o : responsecode.c responsecode.h
	gcc -g -c responsecode.c
client.o : client.c $(configs)
	gcc -g -c client.c
.PHONY : clean
clean :
	-rm client $(object)
