objects = sws.c connect.c log.c process.c parser.c response.c

make : $(objects)
	cc -g -Wall -Werror -Wextra -lmagic -o sws $(objects)

clean :
	rm a.out sws.o connect.o log.o process.o parser.o
