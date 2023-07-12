# mtkproto / mtkrunner

A tool solely made to upload and run code into the Mediatek SoCs,
with the neccessary facilities to do it more-or-less easily. (or at least it should be like that, right now it really isn't)

If you want to do something else, then take look at [mtkclient](https://github.com/bkerler/mtkclient) or similar.

To better understand why i ever need this, check out the mess on commit [c55c440dff](https://github.com/kagaimiq/mtkproto/tree/c55c440dff2c6d869d0aed4308d4068dc32b0630),
and especially the "payload-mt6580-LinuxBoot" directory, which is a rather simple ARM Linux boot wrapper with mediatek bits bolted on top.
(it [bleeps](https://github.com/kagaimiq/mtkproto/blob/c55c440dff2c6d869d0aed4308d4068dc32b0630/payload-mt6580-LinuxBoot/sperd.c#L675) into your headphones!)

-------------------------------------------------------------------------------------------------------------

It doesn't bypass all the obstacles like DAA or SLA, and so if your device is locked out, or there is any suspicion,
then use something else to bypass it, for example with mtkclient you can run `./mtk payload` and it should bypass it.

Currently it's still has some hardcoded bits (that is, the EMI init data for DA), which should be handed correctly.

Also the Download Agent variant it supports is the "xflash" one, which is used by SoCs like MT6765,
while the option for the older DA variant that is used by SoCs like MT6580 is not currently available.

The reason is really because devices with e.g. MT6580 or MT6735 doesn't really have any problem with the preloader,
as it can appear as an "MT65xx Preloader" device, so I can just load to DRAM directly without any download agents,
while in case of MT6762V/CN (yes, it's *NOT* MT6761, it's a cut-down MT6765!) in my Xiaomi Redmi 6A phone,
it had a secure boot option enabled, and as a consequence, the Preloader also lacked its USB device, too.
So the only option was to use the DA, as the DAA/SLA protections in BROM can be easily bypassed.

## Usage

`./mtkproto <mtk tty> [<payload addr> <payload file> [<payload 2 addr> <payload 2 file>]]`

- `<mtk tty>` - The port where the device resides, e.g. /dev/ttyACM0.
- `<payload addr>` - Address where the first payload is loaded
- `<payload file>` - File that will be loaded as the first payload
- `<payload 2 addr>` - Address where the second payload is loaded
- `<payload 2 file>` - File that will be loaded as the second payload

First thing it does after starting is it waits for the device on the port "`<mtk tty>`" indefinetly until it appears,
and it is able to open it.
Just hold a volume button (pull down KPCOL0 to GND) then plug in into USB, or wait for the preloader device, etc.

If you specified address 0x58881688, then the corresponding file will be intepreted as an Preloader/LK image,
from which the correct address and size is obtained, which might come handy. (although the way to specify that might change!)

If no additional parameters specified (i.e. only the `<mtk tty>` is specified), it just prints out some information stuff obtained from BROM/Preloader mode.

The first payload is loaded in the BROM/Preloader mode via commands `SEND_DA` and `JUMP_DA`.

Then the second payload is loaded in the DA "xflash" protocol, first sending out DRAM configs (hardcoded) and other stuff first.

It's important to know that the DA's first stage (that we are executing now) expects to have its second stage
(that we are supposed to load) loaded, and so it checks the hash (SHA1 or SHA256) of what we have just loaded ("security check"),
and refuses to execute it if the hashes are different from what was hardcoded into DA1 binary itself.

Currently the option is to manually patch out this check, or calculate the correct hash into the DA itself. (whichever you prefer better)

### Example

Run via BROM or Preloader:

* in SRAM: `./mtkproto /dev/ttyACM0 0x200000 payload.bin`
* in DRAM (in preloader): `./mtkproto /dev/ttyACM0 0x80001000 MT6580_LinuxCardReader.bin`

Run something from DRAM via Download Agent:

* `./mtkproto /dev/ttyACM0 0x200000 mtk_DA6765_nohashcheck.bin 0x48000000 linux.bin`

The same but with an LK image instead:

* `./mtkproto /dev/ttyACM0 0x58881688 X2/mtk_DA6765_nohashcheck.img 0x58881688 ../LinuxBootWrap/mt6762m/LinuxImage.img`
