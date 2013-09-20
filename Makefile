CFLAGS = -std=gnu99 -Wall -Wextra -O2 -pedantic

LDFLAGS =
COMMON_SOURCES =
COMMON_OBJECTS = $(COMMON_SOURCES:.c=.o)
INSTALL_PATH = /usr/local/bin
EXECUTABLE = unformatter

all: $(EXECUTABLE)

unformatter: unformatter.o $(COMMON_OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm *.o test* *~ $(EXECUTABLE) -rf

.PHONY: install
install:
	install -m 0755 $(EXECUTABLE) /usr/local/bin

.PHONY: uninstall
uninstall:
	rm -rf /usr/local/bin/$(EXECUTABLE)
