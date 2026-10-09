# SPDX-License-Identifier: GPL-3.0-or-later
# The devicetree preprocessor needs a target the compiler knows (TC32_THUMB:
# a clang for ARMv4T Thumb, see zephyr arch/tc32/CMakeLists.txt).
if(TC32_THUMB)
  list(APPEND DTS_EXTRA_CPPFLAGS --target=thumbv4t-none-eabi)
else()
  list(APPEND DTS_EXTRA_CPPFLAGS --target=tc32-unknown-none-elf)
endif()
