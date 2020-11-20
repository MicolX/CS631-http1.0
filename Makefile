objects = sws.c connect.c log.c process.c

make : $(objects)
	cc -g -Wall -Werror -Wextra -o sws $(objects)

clean :
	rm a.out sws.o connect.o log.o process.o 
