# SPDX-License-Identifier: GPL-3.0-or-later

zephyr_library()
zephyr_library_sources(board.c)
zephyr_library_sources_ifdef(CONFIG_TLSR_BOARD_DEEP_SLEEP_PADS deep_sleep_pads.c)
# This directory on the include path: led_driver_table.h and led_driver_status.h (the LED
# wiring and the status part the LED driver includes), v75pro_status.h. The board's part of the status display, v75pro_status.c, is built
# into the ZMK app by tc32/CMakeLists.txt, which takes its name from this property.
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY TC32_BOARD_STATUS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/v75pro_status.c)
# The code run most often first in .text, together in the instruction cache (hot_text.ld).
zephyr_linker_sources(TEXT_SECTIONS hot_text.ld)
