# Release notes

## v0.1.0

ZMK for the CIDOO V75 Pro, V65 V3 and V21 on the Telink TLSR8278, installed over USB with the keyboards' original update protocol. Each keyboard's image is the full build (the `studio` variant of `build.sh`): wired, Bluetooth (three bonded profiles) and the 2.4G dongle link, the backlight with its effects kept over a power cut, ZMK Studio over a USB serial port, US-JIS (V75 Pro, V65 V3), the status display (V75 Pro, V65 V3), updates over USB with the confirm, and the ways back to the original firmware by key, by chord at power-on, or on their own after three unconfirmed boots.

Bluetooth: Secure Connections pairing with Passkey Entry typed on the keyboard; a host that offers only legacy pairing is refused. The keyboard shows up on hosts such as Android phones that list a keyboard only after its scan response. Boot keyboard reports for a computer's BIOS. The host's keyboard LEDs on the backlight: Caps Lock on the V75 Pro and V65 V3, Num Lock on the V21.

Keymaps: `&ext_power`, `&sys_reset` and `&bootloader` are not available (none of the boards has a switchable power rail, and updates go through the update tool, not a bootloader).

Built from zmk-tc32 d2bdf11b0554347f64f609f7c04bf24eb30d55c1 (upstream ZMK main 9ebbeff0 with the TLSR8278 module), zephyr-tc32 a7410dadb103cf00a9e78593a3abada3219257d3 (Zephyr v4.5.0-rc1 with the TC32 port), nanopb 5499fd4c, zmk-studio-messages 6cb4c283, with tc32-devtools 2e1bcaed762e3124b024047db981172d2b0395ba, llvm-tc32 clang 23.1.2 (branch `tc32/23.1.2`, 27606ee6), CMake 4.4.2, Ninja 1.13.2, Python 3.14.6, dtc 1.8.1. The images depend on these sources and tools only, not on a repository's history or tags. `./build.sh BOARD` gives:

| Board | `zmk.bin` | SHA-256 of `zmk.bin` | `zmk.ota.bin` (the image to install) | SHA-256 of `zmk.ota.bin` |
|---|---|---|---|---|
| cidoo_v75pro | 127,852 B | 29aa24c796b6e4da7ad3274037d9c953ecde0fd73af6ef046c3d89bb0c43b912 | 127,860 B | 6b1c84aec798a13ef3898d5cc031dd2fa0ef8dcb9e771a5b85e38d60a42c134c |
| cidoo_v65v3 | 127,484 B | 6bbca3f519805d52d9f7e944ea408a642b9fe77dd4d7c4a82072726569660547 | 127,492 B | ab20dc6cf667952ff8879e7f8f4e9fa37db106d4d11f5ff7636c98d1b6a21d4c |
| cidoo_v21 | 119,292 B | 41485cc6938e7104262727965a1b83c6d1b4bc76cdb5b90f5f60d412f8ea13b7 | 119,300 B | 9fb82509e837fa60b47a8f4768dcea0a643bc31dbbeea2fb866d71b3eaecb9e2 |

The release on GitHub holds one zip per board (`cidoo_v75pro.zip`, `cidoo_v65v3.zip`, `cidoo_v21.zip`), each with the board's `zmk.ota.bin` above and the tool `telink_ota.py` of zmk-tc32 d2bdf11b, as [INSTALL.md](INSTALL.md) expects them in one folder. `DIRECT=1 ./build.sh BOARD` gives the same bytes. Every image passes the build's checks.

On hardware: a V21 ran an earlier build, image 75b6dc83 (zephyr-tc32 0725dd2ffda, zmk-tc32 d5ebd26c, tc32-keyboards 5567225): the install through the original firmware's updater and the ways back, typing over USB, over Bluetooth (a Mac, an iPhone and an Android phone) and over the 2.4G dongle, the Num Lock LED, and ZMK Studio. The fixes made after that image have not run on hardware yet (tested in an emulator only). The V75 Pro and V65 V3 images have not run on hardware yet (tested in an emulator only).

A Bluetooth bond made by legacy pairing (by earlier builds) is not used: the host pairs again.

Known limits: the V21 has no status display; the full builds leave little RAM free (under 1 KB on the V75 Pro and V65 V3, about 1.6 KB on the V21); the 2.4G link does not answer the dongle's VIA requests; there is no low-battery warning (the keyboard shows nothing before it reaches 0 % and goes to sleep). zmk-tc32's `tc32/docs/known-gaps.md` lists the known gaps with what each means for use.
