TARGETS = diffwcx

all: $(TARGETS)

.PHONY: all clean

clean:
	-rm $(TARGETS)
