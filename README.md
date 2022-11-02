# mtkproto

A tool for executing arbitrary code on MediaTek SoCs through the USBDL (BROM/Preloader) and Download Agent protocols.

For bypassing the <del>restrictions</del>, look [there](https://github.com/MTK-bypass)

For doing something else, maybe look at [mtkclient](https://github.com/bkerler/mtkclient), maybe?

## Note

### Download agent

The Download Agent protocol support right now is limited to the MT6765's DA
(probably will work with any other DA that speaks the *same* protocol - e.g. MT6580's DA speaks completely different protocol)

And the DRAM init data is hardcoded to the ones that were in the Preloader from Xiaomi Redmi 6A.

The included `mtk_DA6765_nohashcheck.bin` has the second DA's stage hash checking bypassed,
and thus the DA from e.g. `MTK_AllInOne_DA.bin` won't work as it will refuse to execute
anything but its second stage.

## Usage

`./mtkproto <mtk tty> [<payload addr> <payload file> [<payload 2 addr> <payload 2 file>]]`

- `<mtk tty>` - The port where the device resides, e.g. /dev/ttyACM0.
- `<payload addr>` - Address where the first payload is loaded
- `<payload file>` - File that will be loaded as the first payload
- `<payload 2 addr>` - Address where the second payload is loaded
- `<payload 2 file>` - File that will be loaded as the second payload

The first payload is loaded on the USBDL (BROM/Preloader) stage,
while the second one is loaded on the Download Agent stage, that is provided by the first payload.

The USBDL stage either *does* have DRAM (when on Preloader) or *does not* have DRAM (when on BROM),
so the Download Agent is used to init the DRAM when we are running off BROM, while on the Preloader it is essentialy a no-op.

### Example

Run something from SRAM (but also can be used for DRAM when running off Preloader):

`./mtkproto /dev/ttyACM0 0x200000 payload.bin`

Run something from DRAM via Download Agent:

`./mtkproto /dev/ttyACM0 0x200000 mtk_DA6765_nohashcheck.bin 0x48000000 linux.bin`

