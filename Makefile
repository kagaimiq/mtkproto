TARGET=mtkproto
OBJS=brom_thing.o mtk_pl.o

PAYLOAD_LOC=payload-mt6580-LinuxBoot

CFLAGS=-Werror
LDFLAGS=

all: $(TARGET) payload.bin

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)
	rm -f payload.bin
	$(MAKE) -C "$(PAYLOAD_LOC)" clean

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"

payload.bin:
	$(MAKE) -C "$(PAYLOAD_LOC)"
