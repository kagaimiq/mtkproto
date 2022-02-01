TARGET=mtkproto
OBJS=brom_thing.o mtk_pl.o

CFLAGS=-Werror
LDFLAGS=

all: $(TARGET)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
	$(MAKE) -C "$(PAYLOAD_LOC)" clean

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"
