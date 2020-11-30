CC=cc
CFLAGS= -Wall -Werror -Wextra -lmagic
SOURCES := $(wildcard * .c)
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
DEPENDS := $(patsubst %.c,%.d,$(SOURCES))

.PHONY: all clean

all: sws

clean:
        $(RM) $(OBJECTS) $(DEPENDS) sws

sws:  $(OBJECTS)
	    $(CC) $(CFLAGS) $^ -o $@

-include $(DEPENDS)

%.o: %.c Makefile
        $(CC) $(CFLAGS) -MMD -MP -c  $< -o $@



