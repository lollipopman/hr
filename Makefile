all: hr

hr:
	$(CC) hr.c -o hr

clean:
	rm -f hr
