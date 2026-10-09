# Installing the firmware

This guide covers three things on a CIDOO V75 Pro, V65 V3 or V21:
- the first install of this firmware;
- later updates;
- going back to the keyboard's original firmware.

Each step gives the command, the tool's output when the step succeeds, and what other outputs mean. The ways back to the original firmware are listed in [Going back](#going-back-the-paths-and-what-each-needs).

## What the install does to the keyboard

The keyboard's flash holds two firmware slots, A and B; the keyboard starts the one marked bootable. The original firmware's own updater writes an update into the slot it is not running from, and this firmware does the same. The install writes into the slot the original firmware is not in; the original firmware stays in its slot, and three paths start it again (below). Going back to it changes which slot is marked bootable.

What gets written to the flash:
- The install writes only the other slot, and marks it bootable last.
- This firmware, while it runs, writes its own data (Bluetooth bonds, the backlight setting, 2.4G dongle records, ZMK Studio's saved keymap, the boot count) to a storage area of its own.
- Not written: the areas where the original firmware keeps its settings, its pairings and its per-unit data. When the keyboard goes back, the original firmware finds them as they were.

A keyboard is "bricked" when it neither boots a working firmware nor accepts an image over USB. The design guards against single failures:
- the running slot is not written;
- a cut transfer leaves no bootable half image, because the new slot is marked bootable last;
- the way back needs no working USB;
- the boot guard goes back on its own after three boots that nobody confirmed.

What none of this can recover is an image that does not even run its first instructions. The tool refuses an image that is not for the keyboard's model.

## Licence and warranty

The firmware is free software under the GNU General Public License, version 3 or later, and comes without any warranty: sections 15 and 16 of the licence apply ([LICENSE](LICENSE)). The keyboard maker's warranty terms for firmware it did not supply are set by the maker.

## What this guide covers and what it does not

Covered:
- a CIDOO V75 Pro, V65 V3 or V21 running its original firmware;
- a USB cable, on macOS or Linux;
- the images and the tool of one release;
- the paths back.

Not covered, and not detected by the tool:
- A different original firmware or hardware revision (other pins or another flash part). The tool checks the model (the USB product string) and nothing else. `info` reports an unknown flash part after the install.
- The original firmware's own write. The original firmware's updater does the first install. What a cable pulled or power lost during that write leaves behind is up to that updater; it is known to mark the new slot bootable only at the end.
- A wrong image for the model. The tool refuses:
  - an image whose product string is not the keyboard's;
  - an image that holds no model's string, or two models';
  - to send when more than one keyboard with the update interface answers on the same USB ID.

  It cannot tell a V75 Pro from another keyboard that answers "CIDOO V75". Before the first install, it cannot tell whether the other slot holds something unexpected.
- Windows. The tool is Python with the `hid` package and should run on Windows, but the steps here were run on macOS.

## Going back: the paths and what each needs

| Path | When it works | What it needs |
|---|---|---|
| The go-back key (`&prev_fw`), held while the firmware runs, then released | Whenever the keyboard scans keys, whether USB works or not | The key's hold time (table below); the other slot's image checks |
| The chord held while the keyboard powers on | Read early at power-on, before USB starts, so it works even when USB does not, as long as the image starts | Power off (cable out in the wired position), the chord held while the cable goes in, held about two seconds; the other slot's image checks |
| The boot guard | On its own: an image the host has not confirmed goes back to the other slot at its third boot that no host configured and that the firmware did not ask for | Nothing; it cannot be turned off before the confirm |
| The confirm (`telink_ota.py confirm`) | After the tests below; it ends the boot guard's counting | A flash test passed in the same boot, and the other slot's image checks |
| The rescue over USB (`flash <original image> --overwrite-other-slot`) | When the keys do not work but USB does. It writes the original firmware's image into the other slot and boots it | The original firmware's image file, USB |

After going back, the original firmware erases the slot this firmware was in within seconds of booting. From then on this firmware comes back only by a new install. For that reason the sequence below confirms only after the ways back have been tried once.

Per board:

| | V75 Pro | V65 V3 | V21 |
|---|---|---|---|
| Original firmware's USB ID and product string | 320F:5055, `CIDOO V75` | 320F:5055, `CIDOO V65 V3` | 320F:5055, `CIDOO V21` |
| This firmware's USB ID and product string | 1D50:615E, `V75 Pro ZMK` | 1D50:615E, `V65 V3 ZMK` | 1D50:615E, `V21 ZMK` |
| Go-back key while running | Fn + Space held 3 s, then released | Fn + Backspace held 3 s, then released | Fn + KP0 held 15 s, then released |
| Chord at power-on | Fn + Up (↑) | Fn + Up (↑) | Fn + KP- (the numpad's minus) |
| When the chord is read | after the boot is counted | after the boot is counted | before the boot is counted |
| Position for the wired install | mode switch in the wired (middle) position | no mode switch; cable in | mode switch in the wired (middle) position |
| Studio unlock key | Fn + Esc | Fn + Del | Fn + KP. |

- The V21's 15 s hold keeps a finger slipping onto KP0, while Fn + KP1 is held for a pairing, from leaving the firmware.
- The chord's keys do nothing else when they are held through the boot that follows.
- Keymap changes in ZMK Studio keep both ways back only if the go-back key and the Studio unlock key stay bound.

## Requirements

- Computer: Python 3 and the `hid` package (`python3 -m pip install hid`). It needs the hidapi library: on macOS `brew install hidapi`, on Linux the distribution's `libhidapi` package.
  - On macOS the terminal application needs Input Monitoring (System Settings > Privacy & Security > Input Monitoring); without it, the tool cannot open the keyboard's interface.
  - On Linux, access to the HID device usually needs root or a udev rule for 320F:5055 and 1D50:615E.
- Keyboard:
  - Charged; the original firmware's updater can refuse to write on a low battery.
  - Plugged in with its own cable, with the mode switch in the wired position (V75 Pro, V21).
  - No other keyboard of these models connected; the tool sends nothing when two keyboards with the update interface answer on one USB ID.
  - The 2.4G dongle unplugged.
- Files, all from the same release: the image for the board (`zmk.ota.bin`), its SHA-256 from the release notes, and the tool `telink_ota.py`. In a release the tool sits next to the image; in a source tree it is `zmk/tc32/scripts/telink_ota.py`. The commands below assume the image and the tool are in the current folder.
- Building an image yourself: see [README.md](README.md#build).
- Time: about twenty minutes for the first install and its tests. An image left unconfirmed goes back on its own after three unconfirmed boots; the install then starts again from step 3.

In the commands below, `KB` stands for the keyboard's original product string from the table (`CIDOO V75`, `CIDOO V65 V3` or `CIDOO V21`). `ZMK` stands for this firmware's (`V75 Pro ZMK`, `V65 V3 ZMK` or `V21 ZMK`). Both are typed in quotes.

## First install, step by step

### 1. Check the files

```sh
shasum -a 256 zmk.ota.bin
python3 telink_ota.py check zmk.ota.bin
```

Expected: the hash equals the release notes' line for the board, and `check` prints one line such as

```
zmk.ota.bin: OK, 130244 bytes of 130244, 8141 chunks, for the V75 Pro
```

The model at the end of the line is the keyboard's. Other outcomes mean the file is not usable for this keyboard; nothing has been sent to the keyboard at this point:
- a different hash means the file is not the release's;
- a different model means it is another board's image;
- an error from `check` (`no KNLT boot flag`, `CRC-32 …`, `size …`) means the image is damaged.

### 2. Identify the keyboard and rehearse

The original firmware answers `info` with an echo rather than with information, so the tool refuses `info` on it. Identify the keyboard by its USB product string instead: on macOS in System Information > USB (or `system_profiler SPUSBDataType`), on Linux with `lsusb -v` or the kernel log. Expected: a device with vendor ID 320f, product ID 5055 and the board's product string (`KB`).

Then rehearse the install without a device:

```sh
python3 telink_ota.py flash zmk.ota.bin --vid 320f --pid 5055 --product "KB" --dry-run
```

Expected (the V75 Pro as the example):

```
zmk.ota.bin: 8141 chunks to 320f:5055, an image for the V75 Pro
05 ... (five report frames in hex)
```

The dry run opens no device: it checks the image against `--product` and prints the first and last frames. Two messages mean the image and the product string disagree:
- `the image is for the X, and --product 'KB' is the Y's: not sent`;
- `--product 'KB' is no known model's`.

### 3. Install over the original firmware

The keyboard is plugged in, in the wired position, and charged. The transfer takes about 15–20 seconds, and the cable stays in until the tool is done.

```sh
python3 telink_ota.py flash zmk.ota.bin --vid 320f --pid 5055 --product "KB" --yes
```

Expected (the V75 Pro as the example; the numbers are the image's):

```
zmk.ota.bin: 8141 chunks to 320f:5055, an image for the V75 Pro
device: 'CIDOO V75', release 0x0101, interface 2
8141/8141
8141 chunks in 17.0 s; sending the end command
done; the keyboard restarts into the new image
```

- The `device:` line shows the board's product string; the tool refuses any other.
- The keyboard disconnects and comes back within a few seconds as `ZMK` on 1d50:615e. On the V75 Pro and V65 V3 the backlight comes on.
- The `release` value is the original firmware's version.

Other outputs:
- `no interface with output report 5 on 320f:5055 …`: the tool found no keyboard with the update interface under that product string. On macOS this also happens when Input Monitoring is missing; the interfaces it could not open are listed as `could not open`. Nothing was sent.
- `N keyboards on 320f:5055 have the OTA interface (…)`: two keyboards answer. Nothing was sent.
- `product string is 'X', not 'KB'`: the keyboard is not the one named. Nothing was sent.
- `no acknowledgement for index N` or `device refused index N: …` during the transfer: the transfer stopped before the end command. The new slot is not marked bootable, and the original firmware still runs after a power cycle (unplug, plug in). The install can be run again from step 3. A failure that repeats in the same way is a case for [Reporting a problem](#reporting-a-problem).
- The keyboard does not come back on USB within ten seconds after `done`: unplugging and plugging in once is the next step.
  - If the original firmware comes back (`CIDOO …` on 320f:5055), the image did not run far enough, and the boot guard or the original firmware took the keyboard back. The keyboard works with its original firmware. The tool's output is what [Reporting a problem](#reporting-a-problem) needs.
  - If nothing comes back on USB at all, see [If something goes wrong](#if-something-goes-wrong).

### 4. First boot: what `info` says

```sh
python3 telink_ota.py info --vid 1d50 --pid 615e --product "ZMK"
```

Expected (a V75 Pro as the example; the clock, flash ID and CPU figure are the unit's):

```
device: 'V75 Pro ZMK', release 0x0404
system clock: 47994000 Hz (measured against the system timer)
watchdog: capture 732, period 4.00 s
boots counted at this boot without a healthy one: 1
image confirmed by the host: no (only 'confirm' clears the count)
flash: JEDEC ID 0x1360c8; status register 0x0000 at boot, 0x0000 now
running from slot A (0x00000)
other slot: its image checks (size word and CRC-32): the ways back have somewhere to go
update gate: an update over the other slot's image needs flash --overwrite-other-slot
this boot: not a reboot the firmware asked for, into this image
planned-reboot mark (analog 0x3c) at this boot: another image's, left by a reboot into that image and kept through the resets since (a power-on clears it)
CPU left for the lowest-priority thread: 68% over 100 ms
uptime: 12 s
```

The lines:
- `boots counted … : 1` and `image confirmed by the host: no`: the first boot of an unconfirmed image. Two more boots without a confirm and it goes back on its own.
- `other slot: its image checks … the ways back have somewhere to go`: the original firmware is in the other slot and intact. `NO image that checks` means it is not intact. The confirm is refused in that state, and the rescue path in the table applies.
- `running from slot A` or `B`: either is right; the original firmware is in the other one.
- `system clock: … (measured …)`: a value near 48,000,000 Hz. The watchdog line reads `period 4.00 s`.
- `flash: JEDEC ID …; status register …`: the flash part and its write protection. `STILL LOCKED after the boot guard's writes`, or `a part the SDK's tables do not cover: not unlocked`, means the boot guard may not be able to write its counter on this unit. In that state the confirm and the revert are not reliable.
- `a flash write of the boot guard or the revert did not read back in this boot`: the same as the previous item.
- The `planned-reboot mark … another image's` line: normal on the first boot after an install; it is gone after a power cycle.
- `CPU left …`: the processor time left over; a V75 Pro at idle shows about 70 %.

If `info` prints `no interface with output report 5 on 1d50:615e with the product string 'ZMK' (interfaces seen: […])`, the keyboard is not on USB as this firmware.
- On a V75 Pro or V21 whose switch is not in the wired position, USB is not up. The interfaces seen then show a Bluetooth name such as `'V75 Pro ZMK 1'`, or nothing. With the switch in the wired position and the cable plugged in again, USB comes up.
- If the original firmware is on USB instead, see step 3's last item.

### 5. Typing test

A minute of normal typing covers:
- ordinary speed;
- the same key repeated fast;
- two keys pressed nearly together;
- a first key after ten seconds of rest;
- Shift and the modifiers;
- the knob.

Expected: every key once, nothing lost, nothing doubled. On the V75 Pro and V65 V3, Fn + B shows the status display for three seconds (the board's README describes it).

If keys are lost or doubled, or the layout is wrong, the image is not one to confirm. The go-back key in step 6 returns to the original firmware.

### 6. The go-back key

The board's go-back key is held for its hold time (table above), then released. Expected: the keyboard disconnects and comes back as the original firmware (`CIDOO …` on 320f:5055), which the OS's USB listing shows. Then the install is repeated:

```sh
python3 telink_ota.py flash zmk.ota.bin --vid 320f --pid 5055 --product "KB" --yes
python3 telink_ota.py info --vid 1d50 --pid 615e --product "ZMK"
```

Expected: as in steps 3 and 4, `boots counted … : 1` and `other slot: its image checks`.

If the key does nothing after the hold:
- A second try with the time counted is the next step; the V21 needs the full 15 s.
- If that does nothing either, the keyboard still works on this firmware. Step 7 tests the other path.
- If the keyboard comes back as this firmware instead of the original, the other slot was not bootable; `info`'s `other slot` line says so.

### 7. The power-on chord

1. Unplug the cable. In the wired position the keyboard is then off.
2. Hold the chord's two keys.
3. Plug the cable in while holding them.
4. Keep holding for about two seconds, then release.

Expected: the keyboard comes up as the original firmware, which the USB listing shows. Then the install and `info` are repeated as in step 6.

If the keyboard comes up as this firmware, the chord was not read: the keys were pressed too late, or not both of them. The keys have to be down before the cable touches.
- If the chord still does nothing but step 6's key worked, that key and the boot guard remain as ways back.
- If neither step 6 nor step 7 worked, the boot guard takes the keyboard back by itself at its third unconfirmed boot. Each unplug, ten seconds' wait and plug-in is one boot.

### 8. A power cut is a boot

Unplug, wait ten seconds, plug in, and run `info`. Expected: `boots counted at this boot without a healthy one: 2`. That is the counter at work: one more unconfirmed boot takes the keyboard back to the original firmware. The confirm comes next.

### 9. Confirm

```sh
python3 telink_ota.py confirm --vid 1d50 --pid 615e --product "ZMK"
```

The confirm first runs a flash test on the firmware's own storage area (nothing in either firmware slot is touched), so that a flash that refuses writes is found now, not later when a way back needs it. The firmware accepts the confirm only when the test passed and the other slot's image checks.

Expected:

```
flash test: passed (the bond log's two sectors erased, written and read back)
device: 'V75 Pro ZMK', release 0x0404
other slot: its image checks (read by the firmware at the confirm)
boots counted without a healthy one: 2 before, 0 after the confirm; image confirmed: yes
```

After the confirm, the boot guard counts no boot. Power cuts, a sleeping host, a charger, or a KVM switched away no longer add up to a revert. The ways back are the key and the chord.

Other outputs:
- `not confirmed: the other slot holds no image that checks …`: the confirm was not sent, because the firmware would refuse it. The original firmware is not intact in the other slot. The message names the repair: the original image written back over USB, described in the symptom table below.
- `flash test failed: flash verify` or `flash error`: the flash did not take a write, or a read-back differed. The image is not confirmed, and nothing in the slots changed. Going back by key or chord still works. `info`'s flash line is what a report needs.
- `confirm refused: the flash test has not passed in this boot`: the test and the confirm have to be one run; running `confirm` again does both.
- `confirm refused: running slot unclear …`: the firmware cannot tell which slot it runs from. An update from this state is not possible; see [Reporting a problem](#reporting-a-problem).

### 10. After the confirm

```sh
python3 telink_ota.py info --vid 1d50 --pid 615e --product "ZMK"
```

Expected:
- `boots counted … : 0`;
- `image confirmed by the host: yes (the automatic healthy rule applies)`;
- `other slot: its image checks`.

From here the mode switch can be set to Bluetooth or 2.4G.

## Updating to a later image

An update writes the other slot, which holds the original firmware after the first install. Afterwards the other slot holds the previous build of this firmware, and the ways back lead to that build, not to the original firmware.

The firmware refuses to write over a working image in the other slot unless the tool is given `--overwrite-other-slot`:

```sh
shasum -a 256 zmk.ota.bin                       # against the release notes
python3 telink_ota.py check zmk.ota.bin         # names the board's model
python3 telink_ota.py info --vid 1d50 --pid 615e --product "ZMK"      # confirmed, count 0, other slot checks
python3 telink_ota.py flash zmk.ota.bin --vid 1d50 --pid 615e --product "ZMK" --yes --overwrite-other-slot
```

Expected:
1. the `device:` line with `'ZMK'`;
2. `the update may overwrite the other slot's image (--overwrite-other-slot)`;
3. the chunks;
4. `done; the keyboard restarts into the new image`.

Without the flag, the tool stops with `the other slot holds an image that checks: … this update would overwrite it …` and sends nothing.

After the restart the new image is unconfirmed. The same tests apply:
- `info`: count 1, and the other slot checks;
- the typing test;
- the go-back key, which now boots the previous build; the new image installs again from it the same way;
- `confirm`.

The first chunk of an update can take up to three seconds to be acknowledged, while the firmware checks the slots first.

## Going back to the original firmware for good

The go-back key and the chord boot the original firmware without writing anything, as long as it is still in the other slot; the first update replaces it.

Putting the original firmware back into the other slot and booting it takes its image file. `check` has to name the board's model (`OK, … for the V75 Pro`). The running firmware then writes it:

```sh
python3 telink_ota.py check original.bin
python3 telink_ota.py flash original.bin --vid 1d50 --pid 615e --product "ZMK" --yes --overwrite-other-slot
```

Expected:
1. `the update may overwrite the other slot's image`;
2. the chunks;
3. `done; the keyboard restarts into the new image`.

The keyboard comes back as `CIDOO …` on 320f:5055, and the original firmware then erases the slot this firmware was in. This is also the rescue path when the keys do not work but USB does.

## If something goes wrong

| Symptom | What it means | Next |
|---|---|---|
| After the install the keyboard is silent: no USB device, no backlight | The image did not get as far as USB. Either it is still starting, or it reset and the boot guard is counting | Wait ten seconds. Then unplug, wait ten seconds, plug in, and try `info`, up to three times. At the third unconfirmed boot the firmware goes back to the original firmware on its own, which then shows as `CIDOO …`. If nothing shows on USB after three power cycles, the chord (step 7) is the remaining path. A case like this is one for [Reporting a problem](#reporting-a-problem); other images change the state the report would describe |
| The original firmware came back on its own | The boot guard's third unconfirmed boot (three power-ons or resets without a confirm), or the chord held by accident | Nothing is lost. A new install followed by steps 4–9 without a long pause gets the firmware back. If it repeats without three power cycles, `info`'s output from the first boot shows the cause |
| `confirm` says `not confirmed: the other slot holds no image that checks` | The other slot does not hold a complete image: a cut transfer, or something overwrote it | The boot count keeps running while the image is unconfirmed. The repair the message names needs the original firmware's image file: write it with `flash --overwrite-other-slot`, then install this firmware from it and confirm. That replaces the other slot |
| `info` shows `boots counted … : 2` when 1 was expected, or `running from slot B` when A was expected | Each power cut, watchdog reset or unplanned reset is one boot. Which slot the original firmware's updater wrote depends on where it ran from | Not a fault. The confirm is due before the third boot, and the slot only has to be the other one from the original firmware's |
| `info` shows `other slot: NO image that checks` right after the first install | The original firmware's slot does not check | The confirm is refused in this state. The cable stays in, so that no boots are added |
| `info` shows `STILL LOCKED` or `a part the SDK's tables do not cover` | The flash's block protection could not be cleared on this unit | The counter and the revert may not be able to write, so a confirm is not reliable. The go-back key returns to the original firmware |
| The chord does nothing | The keys were not down before power came, they were the wrong keys, or (on the V75 Pro and V65 V3) the firmware asked for this boot, and a chord is not read then | Unplug, hold both keys first, plug in, hold for two seconds. The go-back key is the other path |
| The go-back key does nothing | The hold was too short (3 s, or 15 s on the V21), or the other slot does not check | A longer hold, then release. `info`'s `other slot` line shows the slot's state |
| `info`/`confirm`: `no interface with output report 5 on 1d50:615e …` | USB is not up as this firmware: the switch is not in the wired position (in Bluetooth and 2.4G, the V75 Pro's and V21's USB is power only), the original firmware is running, or macOS Input Monitoring is missing | Wired position, cable plugged in again; the USB listing; Input Monitoring |
| The keys type but USB does not work at all | The go-back key or the chord returns to the original firmware; no tool is needed | — |

## Reporting a problem

Reports are welcome as issues on the repository, but there is no promise that anyone will answer or fix anything (see the notice on this firmware's risk in [README.md](README.md)). A report with the following makes the cause traceable:
- the board;
- the release (the image's SHA-256);
- the step reached;
- the tool's full output;
- the output of `info`;
- if the keyboard runs this firmware, the output of `python3 telink_ota.py link --vid 1d50 --pid 615e --product "ZMK"` (it writes nothing).

## Before the first install

- The keyboard is a CIDOO V75 Pro, V65 V3 or V21, and its USB product string is the one in the table.
- The board's go-back key, its hold time and the chord keys are known and found on the keyboard.
- The image for the board and the tool come from the same release, in one folder, and the image's SHA-256 matches the release notes.
- `telink_ota.py check` names the board's model.
- The keyboard is charged and plugged in with its cable, in the wired position. No other keyboard of these models is connected, and the 2.4G dongle is unplugged.
- On macOS the terminal has Input Monitoring, and the `hid` package imports (`python3 -c "import hid"`).
- About twenty minutes are free, and the cable stays in during each transfer.
- The confirm comes after the go-back key and the chord have each taken the keyboard back once.
