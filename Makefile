objects = sws.c connect.c

make : $(objects)
	cc -Wall -Werror -Wextra -o sws $(objects)

clean :
	rm a.out sws.o connect.o
