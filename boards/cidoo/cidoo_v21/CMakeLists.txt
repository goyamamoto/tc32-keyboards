# SPDX-License-Identifier: GPL-3.0-or-later

zephyr_library()
zephyr_library_sources(board.c)

# The code run most often first in .text, together in the instruction cache (hot_text.ld).
zephyr_linker_sources(TEXT_SECTIONS hot_text.ld)
