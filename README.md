# tc32-keyboards

Open-source firmware for retail keyboards whose makers ship proprietary firmware on Telink's TC32 microcontrollers, or do not publish the source of the firmware they ship. The firmware is [ZMK](https://zmk.dev), built for the Telink TLSR8278 through [zmk-tc32](https://github.com/goyamamoto/zmk-tc32) (ZMK with a TLSR8278 module) and [zephyr-tc32](https://github.com/goyamamoto/zephyr-tc32) (the Zephyr RTOS with a port to Telink's TC32 core). It is installed over the USB cable with the update protocol of each keyboard's original firmware: no case to open, no programmer. The original firmware stays in the keyboard's other flash slot, and a key or the boot guard takes you back to it.

**At your own risk.** This firmware replaces the keyboard maker's firmware. Installing it, using it or going back from it can make the keyboard unusable, lose its settings and pairings, or end the maker's warranty. It comes with no warranty of any kind (see the [licence](#licence)) and with no support: nobody, neither the authors nor CIDOO, Telink, the ZMK or the Zephyr projects, can help you or takes any responsibility for what happens to your keyboard, your computer or your data. The way back to the original firmware described here worked on the keyboards it was tried on; that is not a promise that it will work on yours. If you are not prepared to lose the keyboard, do not install this.

The code is written so that other keyboards on the same chip can be added.

## Supported keyboards

Currently three CIDOO models:

| Board | Keyboard | Links | Keys |
|---|---|---|---|
| [`cidoo_v75pro`](boards/cidoo/cidoo_v75pro/README.md) | CIDOO V75 Pro, 75% ANSI with a knob | USB, Bluetooth (3 profiles), the CIDOO 2.4G dongle, by the mode switch | 82 and the knob |
| [`cidoo_v65v3`](boards/cidoo/cidoo_v65v3/README.md) | CIDOO V65 V3, 65% ANSI with a knob | USB, Bluetooth, 2.4G, chosen by key (no mode switch) | 67 and the knob |
| [`cidoo_v21`](boards/cidoo/cidoo_v21/README.md) | CIDOO V21, a numpad with a knob | USB, Bluetooth, 2.4G, by the mode switch | 21 and the knob |

What the firmware gives on each of them:
- ZMK: layers, hold-taps, macros, the knob as an encoder, and ZMK Studio over a USB serial port to change the keymap without a build; the saved keymap outlives updates.
- Wireless: Bluetooth with three bonded profiles and Secure Connections pairing (Passkey Entry typed on the keyboard), and the 2.4G link to the keyboards' CIDOO USB dongle; sleep on battery; the battery level to the host.
- The backlight with its effects, colours and levels, kept over a power cut, and a status display on the keys (battery, Bluetooth and 2.4G state, Windows/Mac mode) for a few seconds on a key (V75 Pro, V65 V3).
- US-JIS: a US layout typed on a host set to a Japanese keyboard (V75 Pro, V65 V3).
- Updates and the way back: firmware updates over the USB cable, with no programmer and nothing to open, and a way back to the original firmware at every step ([zmk-tc32's documentation](https://github.com/goyamamoto/zmk-tc32/blob/main/tc32/README.md)).

## The images

`./build.sh BOARD` of this tree gives these images; [RELEASE-NOTES.md](RELEASE-NOTES.md) has their full SHA-256 and the commits and tools they were built with.

| Board | `zmk.ota.bin` (the image to install) | SHA-256 starts |
|---|---|---|
| `cidoo_v75pro` | 127,860 B | `6b1c84ae` |
| `cidoo_v65v3` | 127,492 B | `ab20dc6c` |
| `cidoo_v21` | 119,300 B | `9fb82509` |

### What has run on hardware

- V21: an earlier build, image `75b6dc83`, ran on one keyboard: the install through the original firmware's updater and the ways back to the original firmware, typing over USB, over Bluetooth (a Mac, an iPhone and an Android phone) and over the 2.4G dongle, the Num Lock LED, and ZMK Studio. The fixes made after that image have not run on hardware yet (tested in an emulator only).
- V75 Pro and V65 V3: the images above have not run on hardware yet (tested in an emulator only). Earlier builds for these two boards, with legacy Bluetooth pairing, were installed through the same updater and taken back to the original firmware on one keyboard each.

## Install

[INSTALL.md](INSTALL.md) gives the first install step by step with the tool's expected output, then updates and going back to the original firmware. In short: the firmware installs over USB with `telink_ota.py flash` and is confirmed with `telink_ota.py confirm` within three boots, after the ways back have been tried once each; an image that is not confirmed goes back to the original firmware at its third boot.

## Build

The build runs on macOS or Linux with git, Python 3, CMake, Ninja, the devicetree compiler (`dtc`), and clang and lld from [llvm-tc32](https://github.com/goyamamoto/llvm-tc32) (built as below), plus [tc32-devtools](https://github.com/goyamamoto/tc32-devtools). West and the Python packages go into a virtual environment in the workspace, `.venv-zephyr`, which `build.sh` uses. The workspace for release v0.1.0:

```sh
mkdir <workspace> && cd <workspace>
python3 -m venv .venv-zephyr && . .venv-zephyr/bin/activate
pip install west
west init -m https://github.com/goyamamoto/tc32-keyboards --mr v0.1.0 .
west update --narrow -o=--depth=1      # tc32-keyboards/, zmk/, zephyr/, modules/lib/nanopb, modules/zmk-studio-messages
pip install -r zephyr/scripts/requirements-base.txt protobuf grpcio-tools
git clone https://github.com/goyamamoto/tc32-devtools ../tc32-devtools   # or TC32_DEVTOOLS=<path>
git -C ../tc32-devtools checkout 2e1bcaed762e3124b024047db981172d2b0395ba   # the commit RELEASE-NOTES.md names
git init ../llvm-tc32 && git -C ../llvm-tc32 fetch --depth 1 https://github.com/goyamamoto/llvm-tc32 27606ee65ffaa0f41a9a8beaabee654325732b62
git -C ../llvm-tc32 checkout FETCH_HEAD                                   # the llvm-tc32 commit RELEASE-NOTES.md names
cmake -G Ninja -S ../llvm-tc32/llvm -B ../llvm-tc32-build \
  -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_TARGETS_TO_BUILD=ARM \
  -DLLVM_DEFAULT_TARGET_TRIPLE=thumbv4t-none-eabi \
  -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_DOCS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DCMAKE_INSTALL_PREFIX=<prefix> \
  -DLLVM_DISTRIBUTION_COMPONENTS="clang;clang-resource-headers;lld;llvm-ar;llvm-ranlib;llvm-nm;llvm-objcopy;llvm-strip;llvm-objdump;llvm-readobj;llvm-readelf;llvm-size;llvm-symbolizer;llvm-addr2line"
ninja -C ../llvm-tc32-build install-distribution
mkdir -p toolchains && ln -s <prefix> toolchains/thumb-llvm         # <prefix> as an absolute path
```

`--mr v0.1.0` takes the release; without it, `west init` takes `main`. `west update` without `--narrow -o=--depth=1` fetches the full histories (zephyr-tc32's is large).

llvm-tc32 is LLVM 23.1.2 with the changes for the TC32 (zmk-tc32's [TC32.md](https://github.com/goyamamoto/zmk-tc32/blob/main/TC32.md) describes the toolchains). A mainstream clang stops the build with a message; with `CONFIG_TC32_POP_PC_RETURNS=n` in a Kconfig fragment (`EXTRA_CONF=<file.conf> ./build.sh ...`) it builds the images, which are then about 4-5 KB larger.

Then, from `tc32-keyboards/`:

```sh
./build.sh cidoo_v75pro            # the image for a keyboard: <workspace>/build/cidoo_v75pro-studio/zephyr/zmk.ota.bin
./build.sh cidoo_v65v3
./build.sh cidoo_v21
```

`build.sh BOARD` builds the full image (the `studio` variant); a second argument `ble` (the wireless links without ZMK Studio) or `wired` (USB only) gives a smaller build. The outputs are `zmk.ota.bin` (the image to install) and `build-info.json` (the commits, the tools and the image's SHA-256). The same sources and tools give the same bytes, so an image can be checked against [RELEASE-NOTES.md](RELEASE-NOTES.md).

## Links

- [zmk-tc32](https://github.com/goyamamoto/zmk-tc32): ZMK with the TLSR8278 module (`tc32/README.md`: updates, the two slots, BLE and 2.4G, the flash, the status display)
- [zephyr-tc32](https://github.com/goyamamoto/zephyr-tc32): Zephyr with the TC32 port and the TLSR8278 drivers
- [tc32-devtools](https://github.com/goyamamoto/tc32-devtools): the compiler path for the TC32, the emulator and the checks
- [ZMK](https://zmk.dev), [ZMK Studio](https://zmk.dev/docs/features/studio)

## Licence

This repository is free software under the GNU General Public License, version 3 or later ([LICENSE](LICENSE)); the source files carry `SPDX-License-Identifier: GPL-3.0-or-later`, and the documents and the recorded boot-path listings are under the same licence. A firmware image built from it holds zmk-tc32's `tc32/` module (GPL-3.0-or-later), ZMK (MIT), Zephyr (Apache-2.0), nanopb (zlib) and zmk-studio-messages (MIT), and is distributed under GPL-3.0-or-later.

Not in this repository and not under its licence: ZMK, Zephyr, zmk-tc32, zephyr-tc32, nanopb and zmk-studio-messages, which west fetches under their own licences; the compiler (llvm-tc32, Apache-2.0 with LLVM exceptions) and tc32-devtools; the keyboards' original firmware, and the VIA definitions and keymap tables the `gen_*.py` scripts read (their inputs are not in this repository).

### Why these licences

Our aim is simple: good keyboards that people can keep using for a long time.

For that, the people who use a keyboard need four rights:

- **Customisability**: to change it to fit how they work.
- **Transparency**: to check that it does nothing they do not want.
- **Reliability**: to depend on it as a tool, every day, for years.
- **Longevity**: to fix it, also when its maker no longer does.

Source code is what makes these rights real. So the firmware (zmk-tc32, tc32-keyboards) is licensed under the GNU General Public License, version 3 or later. We welcome makers who build better keyboards with this work, and we ask them to develop in the open, so that the people who buy those keyboards keep these rights.
