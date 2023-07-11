TARGET = mtkproto
OBJS   = mtkrunner.o mtk_pl.o mtk_dev.o

CFLAGS  = -Werror
LDFLAGS =

all: $(TARGET)

clean:
	rm -f $(OBJS)
	rm -f $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
