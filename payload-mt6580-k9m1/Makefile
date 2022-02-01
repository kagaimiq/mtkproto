TARGET=mtkpayload
OBJS=\
	pre.o \
	sperd.o \
	com/xprintf.o \
	com/mtk_gpio.o \
	com/mtk_pmic.o \
	com/wallclk.o

CFLAGS=-mcpu=cortex-a7 -Os -Werror -mthumb -Icom
ASFLAGS=-mcpu=cortex-a7
LDFLAGS=-mcpu=cortex-a7 -Tlink.ld -nostartfiles

CROSS_COMPILE=arm-none-eabi-
CC=$(CROSS_COMPILE)gcc
AS=$(CROSS_COMPILE)as
OBJDUMP=$(CROSS_COMPILE)objdump
OBJCOPY=$(CROSS_COMPILE)objcopy
SIZE=$(CROSS_COMPILE)size

all: $(TARGET) copy

clean:
	rm -f $(OBJS)
	rm -f "$(TARGET)"
	rm -f "$(TARGET).bin"
	rm -f "$(TARGET).lst"

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o "$@"
	$(SIZE) -A "$@"

copy: $(TARGET)
	$(OBJDUMP) -d "$<" > "$<.lst"
	$(OBJCOPY) -Obinary "$<" "$<.bin"
	cp "$<.bin" "../payload.bin"
