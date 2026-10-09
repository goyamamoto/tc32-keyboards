/*
 * The code the V21 reads most often from the flash, at the start of .text (TEXT_SECTIONS, CMakeLists.txt): the
 * system timer interrupt and the kernel timeouts it drives, the USB poll the key matrix interrupt calls, the work
 * queue and the switch to the next thread, the BLE link's events and the 2.4G link's radio interrupt and packet
 * exchange. The key matrix interrupt itself runs from RAM (.ram_code). The flash instruction cache has 64 lines of
 * 32 bytes, each address in one line only, and this is more than its 2 KB, so the order decides which of these put
 * one another out of it. The order here is the one with the fewest cache misses over read traces of the wired, BT
 * and 2.4G positions, each idle, with a key held and over a second of typing, at 190 cycles per miss (the wired
 * key-held and the BT typing traces counted three times: an order searched over idle and key-held traces alone
 * lowers the CPU time but lengthens the key-down latency over BT), counted for the image linked in each order (read
 * traces taken in the emulator; the order searched over them). A section is named by its function, so a name takes
 * that function of every object that has one.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

*(.text.arch_cpu_idle) *(.text.sys_clock_set_timeout) *(.text.z_add_timeout)
*(.text.usb_poll) *(.text.b87_stimer_irq) *(.text.z_timer_expiration_handler)
*(.text.sys_clock_announce_locked) *(.text.ble_zmk_take) *(.text.p24_run)
*(.text.z_sched_add_to_waitq_locked) *(.text.work_queue_main) *(.text.tlsr_radio_irq_status)
*(.text.notify_queue_locked) *(.text.z_get_next_switch_handle) *(.text.strip_length)
*(.text.elapsed) *(.text.idle) *(.text.k_uptime_get)
*(.text.z_impl_device_is_ready) *(.text.z_pend_curr) *(.text.submit_to_queue_locked)
*(.text.z_impl_k_mutex_unlock) *(.text.z_impl_k_timer_start) *(.text.event_start)
*(.text.switch_poll) *(.text.z_impl_k_mutex_lock) *(.text.z_swap)
*(.text.sys_clock_tick_get) *(.text.exchange) *(.text.event_end)
*(.text.z_timeout_inflight_superseded) *(.text.in_done) *(.text.reset_isr)
*(.text.p24_rf_isr) *(.text.strip_update_rgb) *(.text.z_work_submit_to_queue)
*(.text.z_impl_k_sem_take) *(.text.z_impl_k_msgq_get) *(.text.rx_packet)
*(.text.__aeabi_uidivmod) *(.text.reprogram_next) *(.text.usb_dc_b87_poll)
*(.text.rf_isr) *(.text.widening) *(.text.fx_work_handler)
*(.text.b87_stimer_program_compare) *(.text.ble_profile_active) *(.text.k_sem_take)
*(.text.z_thread_timeout) *(.text.gpio_b87_pin_configure) *(.text.z_sched_unready_locked)
*(.text.build_report) *(.text.gpio_b87_ie_update) *(.text.tlsr827x_analog_read)
*(.text.tlsr_radio_conn_event_prepare) *(.text.led_key_matrix_hide) *(.text.answered)
*(.text.gpio_b87_pull) *(.text.gpio_pin_configure) *(.text.read_mode_switch)
*(.text.pend_locked) *(.text.__udivsi3) *(.text.tlsr827x_irq_mask_read)
*(.text.tc32_rng_add_event) *(.text.z_swap_locked) *(.text.next_timeout)
*(.text.k_mutex_unlock) *(.text.tlsr827x_analog_write) *(.text.z_impl_k_busy_wait)
*(.text.z_sched_wait) *(.text.ble_radio_give) *(.text.tlsr_radio_tx_fifo_restore)
*(.text.ev_timer_fn) *(.text.wheel) *(.text.work_timeout)
*(.text.z_sched_ready_locked) *(.text.z_try_abort_timeout) *(.text.z_impl_k_sem_give)
*(.text.memcpy) *(.text.cidoo_battery_power_in) *(.text.next_channel)
*(.text.tlsr_radio_set_sn_nesn) *(.text.arm) *(.text.ble_usb_host)
*(.text.ble_battery_poll) *(.text.low_power) *(.text.ble_att_battery_level)
*(.text.memset) *(.text.paint) *(.text.sys_clock_cycle_get_32)
*(.text.gpio_pin_get_dt) *(.text.q_put) *(.text.k_work_schedule_for_queue)
*(.text.k_mutex_lock) *(.text.zmk_keymap_position_state_changed) *(.text.z_tc32_mul32x32)
*(.text.fallback_poll) *(.text.k_busy_wait) *(.text.z_tc32_udiv64_avg_mulhi)
*(.text.tlsr_radio_clear_irq) *(.text.hid_listener) *(.text.unpend_thread_no_timeout)
*(.text.scaled) *(.text.tlsr_radio_set_power_level) *(.text.band_bits)
*(.text.tlsr_radio_set_ble_channel) *(.text.tlsr827x_analog_wait) *(.text.show)
*(.text.z_reschedule) *(.text.crc16) *(.text.ticks_now)
*(.text.ble_conn_wait_ticks) *(.text.zmk_usb_is_hid_ready) *(.text.sys_clock_elapsed)
*(.text.k_msgq_get) *(.text.tlsr_radio_p24_access) *(.text.p24_running)
*(.text.tlsr_radio_now) *(.text.z_tc32_switch) *(.text.z_impl_behavior_get_binding)
*(.text.cidoo_battery_read) *(.text.pins_up) *(.text.tlsr_radio_off)
*(.text.zmk_event_manager_handle_from) *(.text.ccm_block) *(.text.gpio_b87_port_get_raw)
*(.text.k_uptime_get_32) *(.text.schedule) *(.text.tlsr_radio_stop)
/*
 * The hot text's end, an absolute symbol so that no tool takes it for the name of the function after it, and
 * TLSR_HOT_TEXT_PAD unused bytes, which move what is linked after the hot text - the GPIO ports' device
 * structures among it, which the scan reads too - to other cache lines (tc32/scripts/hot_slots.py lists the
 * lines of the scan's code and constants). The bytes are zmk-tc32's tc32/src/tlsr_hot_text_pad.S: data, the same
 * on both build paths.
 */
_tlsr_hot_text_end = ABSOLUTE(.);
KEEP(*(.tlsr_hot_text_pad))
ASSERT(ABSOLUTE(.) - _tlsr_hot_text_end == CONFIG_TLSR_HOT_TEXT_PAD, "the hot text pad is not TLSR_HOT_TEXT_PAD bytes");
