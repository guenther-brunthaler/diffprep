TARGETS = diffprep

all: $(TARGETS)

.PHONY: all clean

AUG_CFLAGS= $(CPPFLAGS) $(CFLAGS)

.c.o:
	$(CC) $(AUG_CFLAGS) -c $<

clean:
	-rm $(TARGETS)
