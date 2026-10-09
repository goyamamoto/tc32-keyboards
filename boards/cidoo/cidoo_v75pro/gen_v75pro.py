#!/usr/bin/env python3
# Copyright (c) 2026 Go Yamamoto
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate the CIDOO V75 Pro ZMK layout and default keymap.

Inputs (not in this repository; pass their paths):
  --via      Cidoo's VIA definition CIDOO_V75PRO-USB.JSON (KLE layout, 8x15 matrix)
  --keymap   a VIA dynamic-keymap dump of a V75 Pro running its original firmware (4 layers x 8 x 15, big-endian
             uint16, VIA protocol 9 keycodes), read with VIA's get_buffer command in
             pieces that stay inside a layer (keymap_2048_aligned.bin): the original firmware
             answers a piece that crosses a layer boundary with 0xffff past it

Outputs (into --out):
  cidoo_v75pro-layouts.dtsi   matrix transform and physical layout
  cidoo_v75pro.keymap         Base and Fn layers taken from original keymap layers 0 and 1
  cidoo_v75pro_ble.keymap     with --ble: the same, and the original firmware's BT channel keys
                              (Fn + Q/W/E) as ZMK profiles for the own BLE stack
                              (TLSR_BLE): a tap selects the profile, held 3 s it selects it
                              and clears its bond (a new pairing), like the channel keys
                              of the original keymap;
                              and the original firmware's 2.4G key (Fn + R) as the 2.4G pairing key
                              (&p24_pair, held 3 s, TLSR_P24)

Matrix row 7 of the VIA definition holds the knob turns (7,0 left, 7,1 right); they
become sensor bindings, not keys. The knob press is the matrix key 0,14.

Going back to the previous firmware: the original firmware's QK_BOOT (Fn + Space) becomes &prev_fw
(held 3 s). The boot guard's power-on chord is Fn + Up (Kconfig.defconfig), keys that do
nothing on the Fn layer, so holding them through the boot has no effect.
"""
import argparse
import json
import os
import re

ROWS, COLS, LAYERS = 8, 15, 4

BASIC = {0x28: "RET", 0x29: "ESC", 0x2A: "BSPC", 0x2B: "TAB", 0x2C: "SPACE", 0x2D: "MINUS",
         0x2E: "EQUAL", 0x2F: "LBKT", 0x30: "RBKT", 0x31: "BSLH", 0x33: "SEMI", 0x34: "SQT",
         0x35: "GRAVE", 0x36: "COMMA", 0x37: "DOT", 0x38: "FSLH", 0x39: "CAPS", 0x49: "INS",
         0x4A: "HOME", 0x4B: "PG_UP", 0x4C: "DEL", 0x4D: "END", 0x4E: "PG_DN", 0x4F: "RIGHT",
         0x50: "LEFT", 0x51: "DOWN", 0x52: "UP", 0xE0: "LCTRL", 0xE1: "LSHFT", 0xE2: "LALT",
         0xE3: "LGUI", 0xE4: "RCTRL", 0xE5: "RSHFT", 0xE6: "RALT", 0xE7: "RGUI",
         # consumer keys in the pre-2023 QMK keycode space used by VIA protocol 9
         0xA8: "C_MUTE", 0xA9: "C_VOL_UP", 0xAA: "C_VOL_DN", 0xAB: "C_NEXT", 0xAC: "C_PREV",
         0xAD: "C_STOP", 0xAE: "C_PP", 0xAF: "C_AL_CCC", 0xB1: "C_AL_EMAIL", 0xB2: "C_AL_CALC",
         0xB4: "C_AC_SEARCH", 0xB5: "C_AC_HOME"}
for i in range(26):
    BASIC[0x04 + i] = chr(ord("A") + i)
for i in range(10):
    BASIC[0x1E + i] = "N" + str((i + 1) % 10)
for i in range(12):
    BASIC[0x3A + i] = "F" + str(i + 1)

# The original keymap's custom keycodes (USER00 = 0x5F80), named as in CIDOO_V75PRO-USB.JSON.
CUSTOM = ["2.4G MODE", "USB MODE", "BT 1", "BT 2", "BT 3", "LOCK WIN KEY", "WIN MODE", "MAC MODE",
          "DESKTOP LIGHT LOW", "DESKTOP LIGHT HIGH", "SWITCH BT", "SWITCH 2.4G", "LED LOGO MODE",
          "LED LOGO COLOUR", "BATT ST"]
RGB = {0x5CC2: "RGB_TOG", 0x5CC3: "RGB_MOD", 0x5CC4: "RGB_RMOD", 0x5CC5: "RGB_HUI",
       0x5CC6: "RGB_HUD", 0x5CC7: "RGB_SAI", 0x5CC8: "RGB_SAD", 0x5CC9: "RGB_VAI",
       0x5CCA: "RGB_VAD", 0x5CCB: "RGB_SPI", 0x5CCC: "RGB_SPD"}


def binding(code, ble=False):
    """ZMK binding and an optional note for one original keycode (ble: the BLE keymap's)."""
    if code == 0x0000:
        return "&none", None
    if code == 0xFFFF:
        raise SystemExit("0xffff in the keymap: a get_buffer read crossed a layer boundary")
    if code == 0x0001:  # KC_TRNS
        return "&trans", None
    if code in BASIC:
        return "&kp " + BASIC[code], None
    if 0x5100 <= code < 0x5110:
        return f"&mo {code & 0xF}", None
    if code in (0x5F10, 0x5F11):  # VIA FN_MO13 / FN_MO23 (tri-layer); plain &mo for now
        return f"&mo {1 if code == 0x5F10 else 2}", None
    if code in RGB:
        return "&none", RGB[code] + " (no RGB yet)"
    if ble and 0x5F82 <= code <= 0x5F84:  # BT 1-3
        n = code - 0x5F82
        return f"&bt_ch {n} {n}", None
    if ble and code == 0x5F80:  # 2.4G MODE: held 3 s in the 2.4G position it pairs
        return "&p24_pair", None
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
    ap.add_argument("--ble", action="store_true", help="also write cidoo_v75pro_ble.keymap")
    a = ap.parse_args()
    via = json.loads(open(a.via, encoding="utf-8-sig").read())
    assert via["matrix"] == {"rows": ROWS, "cols": COLS}, via["matrix"]
    raw = open(a.keymap, "rb").read()
    code = lambda layer, r, c: int.from_bytes(raw[2 * ((layer * ROWS + r) * COLS + c):][:2], "big")

    keys = []
    for label, x, y, w, h in kle_keys(via["layouts"]["keymap"]):
        r, c = (int(v) for v in re.match(r"(\d+),(\d+)", label).groups())
        keys.append((r, c, x, y, w, h))
    matrix_keys = [k for k in keys if k[0] < 6]
    knob = [k for k in keys if k[0] == 7]
    assert len(matrix_keys) == 82 and len(knob) == 2, (len(matrix_keys), len(knob))
    # Fn at 5,10; QK_BOOT (&prev_fw) at Fn + Space (5,6); the power-on chord Fn + Up (4,13) must be
    # &none on the Fn layer (Kconfig.defconfig).
    assert code(0, 5, 10) == 0x5F10 and code(1, 5, 6) == 0x5C00
    assert binding(code(1, 4, 13))[0] == "&none" and code(0, 4, 13) == 0x0052

    u = lambda v: int(round(v * 100))
    t = ["/*", " * CIDOO V75 Pro: matrix transform and physical layout.",
         " * Generated by gen_v75pro.py from Cidoo's CIDOO_V75PRO-USB.JSON.", " */", "",
         "#include <dt-bindings/zmk/matrix_transform.h>", "#include <physical_layouts.dtsi>", "",
         "/ {", "    default_transform: keymap_transform_0 {",
         '        compatible = "zmk,matrix-transform";', "        rows = <6>;", "        columns = <15>;",
         "        map = <"]
    row = None
    for r, c, *_ in matrix_keys:
        if r != row:
            if row is not None:
                t.append(line)
            line, row = "            ", r
        line += f"RC({r},{c}) "
    t += [line, "        >;", "    };", "", "    v75pro_layout: v75pro_layout {",
          '        compatible = "zmk,physical-layout";', '        display-name = "ANSI 75%";',
          "        transform = <&default_transform>;", "        keys  //                     w   h    x    y  rot  rx  ry"]
    for i, (r, c, x, y, w, h) in enumerate(matrix_keys):
        sep = "=" if i == 0 else ","
        t.append(f"            {sep} <&key_physical_attrs {u(w):3d} {u(h):3d} {u(x):4d} {u(y):4d}    0   0   0>"
                 f"  // {r},{c}")
    t += ["            ;", "    };", "};", ""]
    os.makedirs(a.out, exist_ok=True)
    open(os.path.join(a.out, "cidoo_v75pro-layouts.dtsi"), "w").write("\n".join(t))

    def layer(n, name, ble=False):
        out = [f"        {name.lower()}_layer {{", f'            display-name = "{name}";', "            bindings = <"]
        row = None
        notes = []
        for r, c, *_ in matrix_keys:
            v = code(n, r, c)
            # The original upper layers repeat the base keys; a key equal to Base is &trans.
            b, note = ("&trans", None) if n > 0 and v == code(0, r, c) else binding(v, ble)
            if r != row:
                if row is not None:
                    out.append(line.rstrip())
                line, row = "                ", r
            line += f"{b:<15} "
            if note:
                notes.append(f"{r},{c}: {note}")
        out += [line.rstrip(), "            >;",
                "            sensor-bindings = <&inc_dec_kp C_VOL_UP C_VOL_DN>;", "        };"]
        if notes:
            out[1:1] = (["            /* Original bindings left out (matrix row,col):"]
                        + [f"             *   {x}" for x in notes] + ["             */"])
        return out

    left, right = (binding(code(0, 7, c))[0] for c in (0, 1))
    km = ["/*", " * CIDOO V75 Pro default keymap.",
          " * Generated by gen_v75pro.py from the original keymap's layers 0 (Base) and 1 (Fn).",
          f" * Original knob: left = {left}, right = {right}, press = 0,14.", " */", "",
          "#include <behaviors.dtsi>", "#include <dt-bindings/zmk/keys.h>", "", "/ {",
          "    keymap {", '        compatible = "zmk,keymap";', ""]
    head = km
    km = head + layer(0, "Base") + [""] + layer(1, "Fn") + ["    };", "};", ""]
    open(os.path.join(a.out, "cidoo_v75pro.keymap"), "w").write("\n".join(km))
    if a.ble:
        # The keys the BLE keymap takes from the original keymap: BT 1-3 on Fn + Q/W/E, 2.4G on Fn + R.
        assert [code(1, 2, c) for c in (1, 2, 3, 4)] == [0x5F82, 0x5F83, 0x5F84, 0x5F80]
        ble = [head[0], " * CIDOO V75 Pro keymap for the own BLE stack (TLSR_BLE) and the 2.4G link",
               " * (TLSR_P24); the same as cidoo_v75pro.keymap but for the original firmware's BT channel keys",
               " * (Fn + Q/W/E): a tap selects ZMK profile 0/1/2, held 3 s it selects it and clears",
               " * its bond; and the original firmware's 2.4G key (Fn + R): held 3 s in the 2.4G position, it pairs.",
               ] + head[2:]
        i = ble.index("#include <dt-bindings/zmk/keys.h>")
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
                    "        /* The original firmware's 2.4G pairing key, Fn + R held 3 s (the 2.4G position only) */",
                    "        p24_pair: p24_pair {",
                    '            compatible = "zmk,behavior-p24-pair";',
                    "            #binding-cells = <0>;",
                    "            hold-ms = <3000>;",
                    "        };",
                    "        bt_ch: bt_ch {",
                    '            compatible = "zmk,behavior-hold-tap";',
                    "            #binding-cells = <2>;",
                    '            flavor = "tap-preferred";',
                    "            tapping-term-ms = <3000>;",
                    "            bindings = <&bt_new>, <&bt_sel>;",
                    "        };",
                    "    };", ""]
        ble += layer(0, "Base", True) + [""] + layer(1, "Fn", True) + ["    };", "};", ""]
        open(os.path.join(a.out, "cidoo_v75pro_ble.keymap"), "w").write("\n".join(ble))
    print(f"{len(matrix_keys)} keys, knob {left} / {right}")


if __name__ == "__main__":
    main()
