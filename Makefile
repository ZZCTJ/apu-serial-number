PROG = get-serial-number

SOURCE = get-serial-number.c

all: get-serial-number.c
	gcc -Wall -o ${PROG} ${SOURCE}
