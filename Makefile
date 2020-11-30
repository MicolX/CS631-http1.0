CC=	cc
CFLAGS=	-g -Wall -Wextra -Werror 
EXE=	sws
OBJS=	connect.o log.o parser.o process.o response.o sws.o

all: ${EXE}


$(EXE):	${OBJS} 
	${CC} ${LDFLAGS} ${OBJS} -lmagic -o ${EXE}

clean:
	rm -f ${EXE} *.o
