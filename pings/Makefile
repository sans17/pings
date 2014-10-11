RM= rm -rf

CFLAGS_ALL= -trigraphs
CFLAGS_PINGS= -std=c99
DEFINES_PROG= -DO=8192 -Dy=", o[X], sizeof(o[X]));" -Dj="(S = socket(2, 1, 0), &R, sizeof R)" -Dk="close(" -DC="FD_CLR("

CC=gcc

all: prog pings

pings: pings.c
	${CC} ${CFLAGS_ALL} ${CFLAGS_PINGS} pings.c -o pings

prog: prog.c
	${CC} ${CFLAGS_ALL} ${DEFINES_PROG} prog.c -o prog

clean:
	${RM} pings pings.exe prog prog.exe core