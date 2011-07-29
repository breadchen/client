
object = client.c errreport.h config.h

client : $(object)
	gcc -g -o client $(object)
