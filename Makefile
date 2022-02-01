TARGET=mtkproto
OBJS=brom_thing.o mtk_pl.o

CFLAGS=-Werror
LDFLAGS=

all: $(TARGET)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"
