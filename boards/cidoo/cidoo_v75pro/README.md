# CIDOO V75 Pro (`cidoo_v75pro`)

ZMK on the CIDOO V75 Pro, a 75% ANSI keyboard with a knob on the Telink TLSR8278. The firmware installs over the keyboard's USB cable with the update protocol of its original firmware, which stays in the other flash slot as the way back; nothing is opened or soldered. The image to install is the full build ([Build](#build)): wired, Bluetooth and 2.4G through the mode switch, the backlight, ZMK Studio, US-JIS, the status display.

| Part | Setting |
|---|---|
| Matrix | `zmk,kscan-gpio-matrix`, `col2row`, polled; a press counts within about 1 ms of the first scan that sees it |
| Columns | PB5 PB4 PB2 PB1 PB0 PA4 PA3 PA2 PA1 PD5 PD4 PD2 PD1 PD0 PC7 (active low) |
| Rows | PC5 PC4 PC3 PC2 PC1 PC0 (active low, pull-up) |
| Knob | `cidoo,detent-encoder` on PC6 and PB7: one step per detent, volume up and down; the press is mute |
| Backlight | Two LED driver chips: the wave, breathe and light effects, colours and levels, kept over a power cut. Fn + Backspace toggles, Fn + \ steps the effect, Fn + Up/Down the brightness, Fn + Right/Left the colour |
| Caps Lock LED | The host's Caps Lock (over USB, Bluetooth or the 2.4G dongle) lights the Caps keycap white over the backlight's effect, at every brightness level. Nothing is lit while the backlight is off or dark (idle, USB suspend, sleep). With US-JIS, Caps Lock is the left Ctrl key, and the light stays on the keycap labelled Caps |
| Mode switch | `cidoo,mode-switch`: PA0 low is BT, PB6 low 2.4G, neither wired. A move while running restarts the keyboard in the new position |
| Battery | `cidoo,battery`: PB3; PD7 high is power in; with it, PD6 high is charging and low a full battery. `telink_ota.py battery` reads the level over USB |
| USB | In the BT and 2.4G positions the cable brings power only; Fn + U brings USB up for the host tools and ZMK Studio while the keys stay on the radio link, and the three LEDs left of the knob blink blue |
| USB suspend | 8 s after the host suspends the bus the keyboard sleeps; a key wakes the host |
| Sleep | Bluetooth: deep sleep after 20 s of advertising to a bonded host or 60 s of advertising for a pairing with no connection, or at 0 % on battery; a low-power state after 5 minutes connected without a key. 2.4G: a low-power state after 5 minutes without a key, deep sleep after 30 minutes or on the dongle's sleep command. Not while a USB host has the keyboard over the cable. A key, the mode switch or the cable wakes it |
| Flash | 512 KB: two 128 KB firmware slots and this firmware's own data (bonds, backlight setting, dongle records, what Studio saves, the boot count). The areas where the original firmware keeps its settings, pairings and per-unit data are left alone, so going back to it keeps its pairings |
| Updates | Over a second USB HID interface, with the confirm ([Firmware updates over USB](https://github.com/goyamamoto/zmk-tc32/blob/main/tc32/README.md#firmware-updates-over-usb)) |
| Going back | The boot guard (three boots of an unconfirmed image), Fn + Space held 3 s (`&prev_fw`) and Fn + Up held while the keyboard powers on boot the image in the other slot ([The two slots and the way back](https://github.com/goyamamoto/zmk-tc32/blob/main/tc32/README.md#the-two-slots-and-the-way-back)) |
| Studio | ZMK Studio over a USB serial port, in any position of the switch while USB is up; Fn + Esc unlocks it. Keep `&studio_unlock` and `&prev_fw` bound when rebinding keys |
| Layers | Base and Fn (Windows), Mac and Mac Fn: Fn + S turns the Mac layer on, Fn + A off, and the choice is kept over a power cut. On the Mac layers F1-F4 are brightness, Cmd+Tab and Cmd+E, F7-F12 the media keys, Alt and Cmd swapped; with Fn the F row gives F1-F12 |
| US-JIS | Fn + Tab toggles US-JIS (a US layout on a host set to a Japanese keyboard), on the Windows layers only; a change blinks the LEDs left of the knob green (on) or red (off). Caps Lock and the left Ctrl are swapped, and the keys beside Space hold their modifier and tap the IME keys (Alt / Muhenkan and Alt / Henkan on Windows, Cmd / Eisu and Cmd / Kana on the Mac); any of these can be given back in Studio |
| Wireless keys | Fn + Q, W, E: Bluetooth profiles 1-3 (a tap selects the profile, held 3 s it also clears its bond, for a new pairing). Fn + R held 3 s in the 2.4G position: pair with the CIDOO dongle again |
| Pairing | Secure Connections with Passkey Entry: the host shows a passkey, type its digits on the number row or the keypad, then Enter (Backspace takes a digit back, Escape gives the pairing up). A host that offers only legacy pairing is refused |

## The status display

Fn + B shows the keyboard's state on the LEDs for 3 s, whatever the backlight's setting (dark when the backlight is toggled off):
- the battery on the three LEDs left of the knob: a bar of one to three, green from 50 %, orange from 20 %, red under it; blue while charging; all three green with the cable in once charging is complete; and on 1 to 0 one key per ten percent, in the bar's colour;
- the Bluetooth profiles on Q, W and E: in the BT position the selected profile green when connected, white while it advertises for a pairing, orange while it looks for its bonded host; other bonded profiles dim blue;
- the 2.4G link on R: green linked, white pairing, orange looking for the dongle;
- the processor time left over on F1 to F10, one key per ten percent, cyan;
- the Windows or Mac mode on A or S, white.

## Keymap files

- `cidoo_v75pro-layouts.dtsi` and `cidoo_v75pro.keymap` are generated by `gen_v75pro.py` from the keyboard's VIA definition and a VIA keymap read from the keyboard (the inputs are not in this repository). Functions this firmware does not have (the logo LED, the Windows key lock) are `&none`, and the keymap comment lists them.
- `cidoo_v75pro_ble.keymap` adds the wireless keys, `cidoo_v75pro_wireless.keymap` the backlight keys, `cidoo_v75pro_studio.keymap` Studio, the Mac layers, US-JIS and the status key: the image for a keyboard.

## Build

In a workspace set up as [README.md](../../../README.md#build) describes, from `tc32-keyboards/`:

```sh
./build.sh cidoo_v75pro
```

This is the full build, the image to install, in `<workspace>/build/cidoo_v75pro-studio/zephyr/zmk.ota.bin`. `./build.sh cidoo_v75pro ble` gives the wireless build without the backlight and Studio, `./build.sh cidoo_v75pro wired` the wired build. The image must fit a 128 KB slot; `telink_ota.py` refuses a larger one.

## Install

**At your own risk.** This firmware replaces the keyboard maker's firmware. Installing it, using it or going back from it can make the keyboard unusable, lose its settings and pairings, or end the maker's warranty. It comes with no warranty of any kind (see the [licence](../../../README.md#licence)) and with no support: nobody, neither the authors nor CIDOO, Telink, the ZMK or the Zephyr projects, can help you or takes any responsibility for what happens to your keyboard, your computer or your data. The way back to the original firmware described here worked on the keyboards it was tried on; that is not a promise that it will work on yours. If you are not prepared to lose the keyboard, do not install this.

Follow [INSTALL.md](../../../INSTALL.md) step by step; it names this board's strings and keys. In short, with the keyboard plugged in, the mode switch in the wired position:

```sh
python3 ../zmk/tc32/scripts/telink_ota.py check   zmk.ota.bin
python3 ../zmk/tc32/scripts/telink_ota.py flash   zmk.ota.bin --vid 320f --pid 5055 --product "CIDOO V75" --yes
python3 ../zmk/tc32/scripts/telink_ota.py info    --vid 1d50 --pid 615e --product "V75 Pro ZMK"
python3 ../zmk/tc32/scripts/telink_ota.py confirm --vid 1d50 --pid 615e --product "V75 Pro ZMK"
```

Confirm within three boots, after the go-back key (Fn + Space held 3 s, then released) and the power-on chord have each taken the keyboard back to the original firmware once and the firmware was installed again. Charge the keyboard before the first install.
