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
      pm_tables_zen1.c \
      pm_tables_zen2.c \
      pm_tables_zen3.c \
      pm_tables_zen4.c \
      pm_tables_zen5.c \
      readinfo.c \
      metrics.c \
      layout.c \
      box_drawing_ops.c \
      json_ops.c \
      text_ops.c \
      influx_ops.c \
      prometheus_ops.c \
      netdata_ops.c \
      smu_messages.c \
      smu_control.c \
      controls.c \
      # EOL
OBJ = $(SRC:.c=.o)

vpath libsmu.c $(LIBSMU_DIR)
vpath smu_common.c $(LIBSMU_DIR)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJ)
