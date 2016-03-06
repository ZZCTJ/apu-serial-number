PROG = get-serial-number

SOURCE = get-serial-number.c

DEFINES = \
	  -DPROGNAME=\"$(PROG)\"


all: get-serial-number.c
	$(CC) -Wall -g -o $(PROG) $(SOURCE) $(DEFINES) $(CFLAGS) $(CFLAGS_EXTRA)

clean:
	rm ${PROG}
