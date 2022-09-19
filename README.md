# mtkproto

**I might abandon this in favor of [mtkclient](https://github.com/bkerler/mtkclient) when i'll make it suitable for my needs....**

Anyway it's far more convenient by itself (e.g. the exploit is applied automatically, and the device is detected automatically too),
all is left over is to make/find a convenient way to load arbitrary code into DRAM in all cases (i.e. by utilizing the Download Agent's first stage),
so that development of the support in e.g. Linux and U-Boot will be really convenient as you are in free enviroment,
not restricted by the ATF, not by Little Kernel...
And you have less potential to screw up the eMMC flash by constant reflashes (but the case when your payload screwed it up is not counted)

**And!** also you can do bizzare stuff like keeping your phone an completely ordinary Android phone...
(maybe the SD card you have in it might look unordinary with the ext4/f2fs/etc filesystems instead of ordinary FAT32/exFAT)
But then you grab your USB cable, connect it, execute the special commands and then your phone now runs Linux!!

Not convinced? oh well....

----

Right now it has no sense IMO, what i'm looking for is a tool that also speaks the
Download Agent protocol since you know, initing the DRAM is *hard*.
And the Preloader might not always have the USBDL mode, e.g my Xiaomi Redmi 6A phone does not.
Probably it's related to the fact that there is *restrictions* all over the place...

----

Note that it does not bypass the DA/serial link auth!
[Look here instead](https://github.com/MTK-bypass)

## Usage
The usage is as follows: `mtkproto <mtk tty> [<payload addr> <payload file>]`,

where the mtk tty is path of the tty on which the device was detected on (e.g. /dev/ttyACM0).
Then the tool will print the device info, and if you specified payload address and file, it will upload it.

The tool will wait indefinetly for the device to be plugged in, so that you can run this tool and then put your device
into the BootROM mode by holding VOL-/VOL+ (pull down KPCOL0) or wait for the Preloader device appear if your device's preloader has the USBDL enabled.
(i think it is case when there is no secure boot or DA/serial link auth or on "simple" socs like MT6580/MT6735)

## Requirements
This tool communicates with the device using POSIX tty API so it will only work on systems that supports it (e.g. Linux).
Apart of that there is no other specific requirements.
