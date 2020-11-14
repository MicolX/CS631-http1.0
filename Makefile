objects = connect.c sws.c

sws : $(objects)
        cc -Wall -Werror -Wextra -o sws $(objects)

clean :
        rm a.out sws.o print.o sort.o