#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# build.sh BOARD [VARIANT]: build the image of BOARD (cidoo_v75pro, cidoo_v65v3, cidoo_v21) with
# zmk-tc32's tc32/scripts/build.sh. VARIANT: studio (the default: the image for a keyboard, with
# the wireless links, the backlight, ZMK Studio and the rest), ble (the wireless links, no Studio)
# or wired. The workspace is this repository's parent, or $TC32_WORKSPACE: zmk/ (zmk-tc32),
# zephyr/ (zephyr-tc32), modules/lib/nanopb and modules/zmk-studio-messages (west update puts them
# there), toolchains/ and .venv-zephyr/ (README.md, Build); tc32-devtools next to the workspace or
# at $TC32_DEVTOOLS. THUMB=1 unless set otherwise; DIRECT=1 takes the direct path (zmk-tc32's
# tc32/scripts/build.sh), which gives the same images. The outputs go to
# <workspace>/build/BOARD-VARIANT/zephyr (or $BUILD_DIR/zephyr): zmk.elf, zmk.bin, zmk.ota.bin
# (the image telink_ota.py flash installs) and build-info.json. $EXTRA_CONF adds Kconfig fragments.
set -u
K=$(cd "$(dirname "$0")" && pwd)
W=$(cd "${TC32_WORKSPACE:-$K/..}" && pwd) || exit 1
board=${1:-cidoo_v75pro}
variant=${2:-studio}
bd=$K/boards/cidoo/$board
[ -d "$bd" ] || { echo "no board $board under $K/boards/cidoo"; exit 1; }
case $board-$variant in
  cidoo_v21-studio) conf="$bd/cidoo_v21_ble.conf;$bd/cidoo_v21_studio.conf" keymap=$bd/cidoo_v21_studio.keymap;;
  *-studio) conf="$bd/${board}_wireless.conf;$bd/${board}_studio.conf" keymap=$bd/${board}_studio.keymap;;
  *-ble) conf="$bd/${board}_ble.conf" keymap=$bd/${board}_ble.keymap;;
  *-wired) conf="" keymap=$bd/$board.keymap;;
  *) echo "variant: studio, ble or wired"; exit 1;;
esac
[ -n "${EXTRA_CONF:-}" ] && conf="${conf:+$conf;}$EXTRA_CONF"
modules=""
[ "$variant" = studio ] && modules="${EXTRA_MODULES:-$W/modules/lib/nanopb;$W/modules/zmk-studio-messages}"
# The recorded instructions of the code executed from reset until the boot is counted and the power-on
# chord can take the keyboard back to the other slot (the boot guard's early stage), which the build is
# checked against (zmk-tc32 tc32/scripts/boot_path_check.py), where the board has them for the variant.
ref=$bd/${board}_${variant}_boot_path.txt
[ -f "$ref" ] || ref=""
THUMB=${THUMB:-1} TC32_WORKSPACE=$W TC32_BOARDS=$K BUILD_DIR=${BUILD_DIR:-$W/build/$board-$variant} \
  EXTRA_CONF="$conf" KEYMAP=$keymap EXTRA_MODULES="$modules" BOOT_PATH_REF=$ref \
  exec sh "$W/zmk/tc32/scripts/build.sh" "$board"
