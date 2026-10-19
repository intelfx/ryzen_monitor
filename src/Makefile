CC ?= gcc
CFLAGS ?= -O3 -mtune=native -march=native -DNDEBUG

LIBSMU_DIR ?= lib

override CFLAGS += -I$(LIBSMU_DIR)
LDLIBS = -lm

OUT = ryzen_monitor
SRC = \
      ryzen_monitor.c \
      libsmu.c \
      smu_common.c \
      pm_tables.c \
      readinfo.c \
      metrics.c \
      layout.c \
      box_drawing_ops.c \
      json_ops.c \
      # EOL
OBJ = $(SRC:.c=.o)

vpath libsmu.c $(LIBSMU_DIR)
vpath smu_common.c $(LIBSMU_DIR)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ)
