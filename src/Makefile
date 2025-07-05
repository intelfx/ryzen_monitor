CC ?= gcc
CFLAGS ?= -O3 -mtune=native -march=native

override CFLAGS += -Ilib
LDLIBS = -lm

OUT = ryzen_monitor
SRC = \
      ryzen_monitor.c \
      lib/libsmu.c \
      pm_tables.c \
      readinfo.c \
      # EOL
OBJ = $(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -rf *.o lib/*.o
