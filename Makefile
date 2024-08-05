ARMCC := /arrc-tool/arrc-am574x-build-env/bin/arm-linux-gnueabihf-gcc
ARM_TRACER_SRC := arm32-tracer.c
ARM_TRACEE_SRC := arm32-tracee.c
ARM_OBJS := arm32-tracer arm32-tracee


all: $(ARM_OBJS)

arm32-tracee: $(ARM_TRACEE_SRC)
	$(ARMCC) arm32-tracee.c -o $@ -Wall
	sshpass -e scp ./$@ fcu1:/root

arm32-tracer: $(ARM_TRACER_SRC)
	$(ARMCC) arm32-tracer.c -o $@ -Wall
	sshpass -e scp ./$@ fcu1:/root

tracer:
	$(CC) tracer.c -o tracer -Wall

tracee:
	$(CC) tracee.c -o tracee -Wall

clean:
	rm -rf tracer tracee $(ARM_OBJS)

.PHONY: clean
