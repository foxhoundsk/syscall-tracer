all: tracer tracee

tracer:
	$(CC) tracer.c -o tracer -Wall

tracee:
	$(CC) tracee.c -o tracee -Wall

clean:
	rm -rf tracer tracee

.PHONY: clean
