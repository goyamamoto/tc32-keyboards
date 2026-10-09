#!/usr/bin/env python3
# Copyright (c) 2026 Go Yamamoto
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the CIDOO V21 ZMK layout and default keymap.

Inputs (not in this repository; pass their paths):
  --via      Cidoo's VIA definition CIDOO_V21_V3-USB.JSON (KLE layout, 4x12 matrix)
  --keymap   the default keymap table: 4 layers x 4 rows x 12 columns of big-endian
             uint16 (VIA protocol 9 keycodes), 384 bytes from the start of the file. A
             keymap read from a unit over VIA is not used: entries never written to the
             unit's settings flash read 0xffff there.

Outputs (into --out):
  cidoo_v21-layouts.dtsi   matrix transform and physical layout
  cidoo_v21.keymap         Base and Fn layers taken from original keymap layers 0 and 1
                           (layers 2 and 3 repeat layer 0)
  cidoo_v21_ble.keymap     with --ble: the same, and the original firmware's BLE channel keys
                           (Fn + KP1/KP2/KP3) as ZMK profiles for the own BLE stack
                           (TLSR_BLE): a tap selects the profile, held 3 s it selects it
                           and clears its bond (a new pairing), like the channel keys of
                           the original keymap

Keys sit on two rows (PC2, PC1). Matrix row 3 of the VIA definition holds the
knob turns (3,0 left, 3,1 right); they become sensor bindings, not keys. The knob
press is the matrix key 1,11.

Going back to the previous firmware: the original firmware's QK_BOOT (Fn + KP0) becomes &prev_fw
(held 3 s). The boot guard's power-on chord is Fn + KP- (Kconfig.defconfig). On the Fn
layer those keys at most dim the backlight one step (RGB_BRD, the original keymap's RGB_VAD
there), so holding them through the boot does nothing else.

The original keymap's backlight keys become ZMK's RGB commands on &bl, the V21's backlight
effects (tc32/src/led_key_matrix_effects.c), with the meanings those keys have on the keyboard.
"""
import argparse
import json
import os
import re

ROWS, COLS, LAYERS = 4, 12, 4
SCANNED_ROWS = 2

BASIC = {0x28: "RET", 0x29: "ESC", 0x2A: "BSPC", 0x2B: "TAB", 0x2C: "SPACE",
         0x53: "KP_NUMLOCK", 0x54: "KP_DIVIDE", 0x55: "KP_MULTIPLY", 0x56: "KP_MINUS",
         0x57: "KP_PLUS", 0x58: "KP_ENTER", 0x63: "KP_DOT", 0x85: "KP_COMMA",
         # consumer keys in the pre-2023 QMK keycode space used by VIA protocol 9
         0xA8: "C_MUTE", 0xA9: "C_VOL_UP", 0xAA: "C_VOL_DN"}
for i in range(9):
    BASIC[0x59 + i] = "KP_N" + str(i + 1)
BASIC[0x62] = "KP_N0"

# The original keymap's custom keycodes (USER00 = 0x5F80), named as in CIDOO_V21_V3-USB.JSON.
# USER00-03 (0x5f80-0x5f83) are 2.4G pairing (held 3 s) and BLE channels 1-3, in that
# order; the VIA definition's names for them are shifted by one.
CUSTOM = ["2.4G pairing", "BT channel 1", "BT channel 2", "BT channel 3"]
# The original keymap's QMK RGB keycodes and ZMK's RGB commands for them (&bl).
RGB = {0x5CC2: "RGB_TOG", 0x5CC3: "RGB_EFF", 0x5CC4: "RGB_EFR", 0x5CC5: "RGB_HUI",
       0x5CC6: "RGB_HUD", 0x5CC7: "RGB_SAI", 0x5CC8: "RGB_SAD", 0x5CC9: "RGB_BRI",
       0x5CCA: "RGB_BRD", 0x5CCB: "RGB_SPI", 0x5CCC: "RGB_SPD"}


def binding(code, ble=False):
    """ZMK binding and an optional note for one original keycode (ble: the BLE keymap's)."""
    if code == 0x0000:
        return "&none", None
    if code == 0xFFFF:
        raise SystemExit("0xffff in the keymap table")
    if code == 0x0001:  # KC_TRNS
        return "&trans", None
    if code in BASIC:
        return "&kp " + BASIC[code], None
    if 0x5100 <= code < 0x5110:
        return f"&mo {code & 0xF}", None
    if code in (0x5F10, 0x5F11):  # VIA FN_MO13 / FN_MO23 (tri-layer); plain &mo for now
        return f"&mo {1 if code == 0x5F10 else 2}", None
    if code in RGB:
        return "&bl " + RGB[code], None
    if ble and 0x5F81 <= code <= 0x5F83:
        n = code - 0x5F81
        return f"&bt_ch {n} {n}", None
    if 0x5F80 <= code < 0x5F80 + len(CUSTOM):
        return "&none", CUSTOM[code - 0x5F80] + " (original firmware only)"
    if code == 0x5C00:  # QK_BOOT: here, back to the firmware in the other slot (held 3 s)
        return "&prev_fw", None
    return "&none", f"original 0x{code:04x}"


def kle_keys(rows):
    """(label, x, y, w, h) from a VIA/KLE keymap, props objects merged until the next key."""
    keys, y = [], 0.0
    for row in rows:
        x, props = 0.0, {}
        for item in row:
            if isinstance(item, dict):
                props.update(item)
                continue
            x += props.get("x", 0.0)
            y += props.get("y", 0.0)
            w, h = props.get("w", 1.0), props.get("h", 1.0)
            keys.append((item.split("\n")[0], x, y, w, h))
            x += w
            props = {}
        y += 1.0
    return keys


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--via", required=True)
    ap.add_argument("--keymap", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--ble", action="store_true", help="also write cidoo_v21_ble.keymap")
    a = ap.parse_args()
    via = json.loads(open(a.via, encoding="utf-8-sig").read())
    assert via["matrix"] == {"rows": ROWS, "cols": COLS}, via["matrix"]
    raw = open(a.keymap, "rb").read()[:2 * LAYERS * ROWS * COLS]
    code = lambda layer, r, c: int.from_bytes(raw[2 * ((layer * ROWS + r) * COLS + c):][:2], "big")
    # Two sanity checks that the file holds the table: ESC at 0,0 and the knob turns.
    assert code(0, 0, 0) == 0x0029 and (code(0, 3, 0), code(0, 3, 1)) == (0x00AA, 0x00A9)
    for n in (2, 3):
        assert all(code(n, r, c) == code(0, r, c) for r in range(ROWS) for c in range(COLS)), n

    keys = []
    for label, x, y, w, h in kle_keys(via["layouts"]["keymap"]):
        r, c = (int(v) for v in re.match(r"(\d+),(\d+)", label).groups())
        keys.append((r, c, x, y, w, h))
    # Transform order is the physical order (top to bottom, left to right), as in VIA.
    matrix_keys = [k for k in keys if k[0] < SCANNED_ROWS]
    assert len(matrix_keys) == 21 == len(keys), (len(matrix_keys), len(keys))
    # Fn at 0,1; QK_BOOT (&prev_fw) at Fn + KP0; the power-on chord Fn + KP- (1,3) may do
    # no more than dim the backlight on the Fn layer (Kconfig.defconfig).
    assert code(0, 0, 1) == 0x5F10 and code(1, 1, 7) == 0x5C00
    assert binding(code(1, 1, 3))[0] in ("&none", "&bl RGB_BRD") and code(0, 1, 3) == 0x0056
    lines = sorted({round(y, 2) for _, _, _, y, _, _ in matrix_keys})

    u = lambda v: int(round(v * 100))
    t = ["/*", " * CIDOO V21: matrix transform and physical layout.",
         " * Generated by gen_v21.py from Cidoo's CIDOO_V21_V3-USB.JSON.", " */", "",
         "#include <dt-bindings/zmk/matrix_transform.h>", "#include <physical_layouts.dtsi>", "",
         "/ {", "    default_transform: keymap_transform_0 {",
         '        compatible = "zmk,matrix-transform";', f"        rows = <{SCANNED_ROWS}>;",
         f"        columns = <{COLS}>;", "        map = <"]
    for ly in lines:
        t.append("            " + " ".join(f"RC({r},{c})" for r, c, x, y, *_ in matrix_keys
                                           if round(y, 2) == ly))
    t += ["        >;", "    };", "", "    v21_layout: v21_layout {",
          '        compatible = "zmk,physical-layout";', '        display-name = "Numpad";',
          "        transform = <&default_transform>;",
          "        keys  //                     w   h    x    y  rot  rx  ry"]
    for i, (r, c, x, y, w, h) in enumerate(matrix_keys):
        sep = "=" if i == 0 else ","
        t.append(f"            {sep} <&key_physical_attrs {u(w):3d} {u(h):3d} {u(x):4d} {u(y):4d}"
                 f"    0   0   0>  // {r},{c}")
    t += ["            ;", "    };", "};", ""]
    os.makedirs(a.out, exist_ok=True)
    open(os.path.join(a.out, "cidoo_v21-layouts.dtsi"), "w").write("\n".join(t))

    def layer(n, name, ble=False):
        out = [f"        {name.lower()}_layer {{", f'            display-name = "{name}";',
               "            bindings = <"]
        notes = []
        for ly in lines:
            line = "               "
            for r, c, x, y, *_ in matrix_keys:
                if round(y, 2) != ly:
                    continue
                v = code(n, r, c)
                if n > 0 and v == code(0, r, c):
                    # The original upper layers repeat the base keys; a key equal to Base is &trans.
                    b, note = "&trans", None
                else:
                    b, note = binding(v, ble)
                line += f" {b:<16}"
                if note:
                    notes.append(f"{r},{c}: {note}")
            out.append(line.rstrip())
        out += ["            >;",
                "            sensor-bindings = <&inc_dec_kp C_VOL_UP C_VOL_DN>;", "        };"]
        if notes:
            out[1:1] = (["            /* Original bindings left out (matrix row,col):"]
                        + [f"             *   {x}" for x in notes] + ["             */"])
        return out

    left, right = (binding(code(0, 3, c))[0] for c in (0, 1))
    unused = [f"{r},{c} {binding(code(0, r, c))[0]}" for r in range(SCANNED_ROWS) for c in range(COLS)
              if code(0, r, c) and not any(k[:2] == (r, c) for k in matrix_keys)]
    km = ["/*", " * CIDOO V21 default keymap.",
          " * Generated by gen_v21.py from the original keymap's layers 0 (Base) and 1 (Fn).",
          f" * Original knob: left = {left}, right = {right}, press = 1,11.",
          " * Original keymap positions not in the VIA layout (no key on the board): " + ", ".join(unused) + ".",
          " */", "",
          "#include <behaviors.dtsi>", "#include <dt-bindings/zmk/keys.h>",
          "#include <dt-bindings/zmk/rgb.h>", "", "/ {",
          "    keymap {", '        compatible = "zmk,keymap";', ""]
    head = km
    km = head + layer(0, "Base") + [""] + layer(1, "Fn") + ["    };", "};", ""]
    open(os.path.join(a.out, "cidoo_v21.keymap"), "w").write("\n".join(km))
    if a.ble:
        ble = [head[0], " * CIDOO V21 keymap for the own BLE stack (TLSR_BLE); the same as",
               " * cidoo_v21.keymap but for the original firmware's BLE channel keys (Fn + KP1/KP2/KP3):",
               " * a tap selects ZMK profile 0/1/2, held 3 s it selects it and clears its bond.",
               ] + head[2:]
        i = ble.index("#include <dt-bindings/zmk/rgb.h>")
        ble[i + 1:i + 1] = ["#include <dt-bindings/zmk/bt.h>"]
        i = ble.index("    keymap {")
        ble[i:i] = ["    macros {",
                    "        bt_sel: bt_sel {",
                    '            compatible = "zmk,behavior-macro-one-param";',
                    "            #binding-cells = <1>;",
                    "            bindings = <&macro_param_1to2>, <&bt BT_SEL MACRO_PLACEHOLDER>;",
                    "        };",
                    "        bt_new: bt_new {",
                    '            compatible = "zmk,behavior-macro-one-param";',
                    "            #binding-cells = <1>;",
                    "            bindings = <&macro_param_1to2>, <&bt BT_SEL MACRO_PLACEHOLDER>, <&bt BT_CLR>;",
                    "        };",
                    "    };", "",
                    "    behaviors {",
                    "        bt_ch: bt_ch {",
                    '            compatible = "zmk,behavior-hold-tap";',
                    "            #binding-cells = <2>;",
                    '            flavor = "tap-preferred";',
                    "            tapping-term-ms = <3000>;",
                    "            bindings = <&bt_new>, <&bt_sel>;",
                    "        };",
                    "    };", ""]
        ble += layer(0, "Base", True) + [""] + layer(1, "Fn", True) + ["    };", "};", ""]
        open(os.path.join(a.out, "cidoo_v21_ble.keymap"), "w").write("\n".join(ble))
    print(f"{len(matrix_keys)} keys, knob {left} / {right}; not in the layout: {', '.join(unused)}")


if __name__ == "__main__":
    main()
