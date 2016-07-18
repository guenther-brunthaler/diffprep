TARGETS = diffprep

all: $(TARGETS)

.PHONY: all clean

clean:
	-rm $(TARGETS)
