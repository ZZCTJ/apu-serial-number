PROG = get-serial-number

SOURCE = get-serial-number.c

all: get-serial-number.c
	$(CC) -Wall -g -o $(PROG) $(SOURCE) $(CFLAGS) $(CFLAGS_EXTRA)

clean:
	rm ${PROG}
