# mtkproto
Tool to upload code binaries into Mediatek SoCs via the BootROM/Preloader protocol

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
