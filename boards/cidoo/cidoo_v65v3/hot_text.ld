/*
 * The code and constants the V65 V3 reads most often, at the start of .text (TEXT_SECTIONS, CMakeLists.txt): the
 * 1 ms USB poll from Timer1's interrupt, the system timer interrupt and the kernel timeouts it drives, the work
 * queue and the switch to the next thread, the key scan with the constants it reads for every output it drives,
 * and the 2.4G link's radio interrupt. The flash instruction cache has 64 lines of 32 bytes, each address in one
 * line only, and this is more than its 2 KB, so the order decides which of these put one another out of it.
 * The order here is the one with the fewest cache misses over read traces of the wired, BT and 2.4G links, each
 * idle, with a key held and over a second of typing, at 190 cycles per miss (the wired key-held and the BT typing
 * traces counted three times: an order searched over idle and key-held traces alone lowers the CPU time but
 * lengthens the key-down latency over BT), counted for the image linked in each order (read traces taken in the
 * emulator; the order searched over them). A section is named by its function, so a name takes that function of
 * every object that has one.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

*(.text.ble_smp_poll) *(.text.zmk_debounce_is_pressed) *(.text.tlsr_radio_p24_channel)
*(.text.ble_radio_take) *(.text.ble_zmk_take) *(.text.k_uptime_get_32)
*(.text.tlsr_radio_conn_timing) *(.text.tlsr_radio_start_conn_event) *(.text.z_timer_expiration_handler)
*(.text.atomic_get) *(.text.sys_clock_announce_locked) *(.text.b87_stimer_program_compare)
*(.text.next_timeout) *(.text.z_get_next_switch_handle) *(.text.sys_clock_set_timeout)
*(.text.reprogram_next) *(.text.gpio_b87_port_clear_bits_raw) *(.text.sys_clock_tick_get)
*(.text.tlsr_radio_off) *(.text.rf_isr) *(.text.z_impl_atomic_set)
*(.text.z_try_abort_timeout) *(.text.z_add_timeout) *(.text.z_swap_locked)
*(.text.memset) *(.text.z_sched_ready_locked) *(.text.u32_bit0_set)
*(.text.usb_poll_timer_isr) *(.text.kscan_matrix_work_handler) *(.text.ble_low_power_poll)
*(.text.chip_end) *(.text.submit_to_queue_locked) *(.text.work_timeout)
*(.text.kscan_gpio_pin_get) *(.text.work_queue_main) *(.text.z_tc32_udiv64_avg_mulhi)
*(.text.ble_hid_leds) *(.text.zmk_debounce_update) *(.text.zmk_debounce_is_active)
*(.text.ticks_now) *(.text.elapsed) *(.text.schedule_for_queue_locked)
*(.text.sys_clock_elapsed) *(.text.k_work_reschedule_for_queue) *(.text.gpio_b87_port_set_bits_raw)
*(.text.tlsr_radio_sn_nesn) *(.text.tlsr_radio_rx_timestamp) *(.text.p24_running)
*(.text.z_pend_curr) *(.text.chip_begin) *(.text.z_swap)
*(.text.p24_rf_isr) *(.text.ble_profile_state_get) *(.rodata.gpio_b87_config_2)
*(.text.gpio_pin_set_dt) *(.rodata.gpio_b87_config_0) *(.rodata.gpio_b87_config_1)
*(.text.gpio_b87_port_get_raw) *(.rodata.kscan_matrix_config_0) *(.text.reset_isr)
*(.text.zmk_debounce_get_changed) *(.text.__udivsi3) *(.text.tlsr_radio_conn_event_prepare)
*(.text.zmk_hid_keyboard_is_pressed) *(.text.in_done) *(.text.gpio_b87_ie_update)
*(.text.z_tc32_mul32x32) *(.text.usb_poll) *(.text.kscan_matrix_read_end)
*(.text.gpio_port_get) *(.text.check) *(.text.z_thread_timeout)
*(.text.link_secure) *(.text.tlsr_radio_set_ble_channel) *(.text.as_zmk_position_state_changed)
*(.text.requests_pending) *(.text.read_mode_switch)
#ifndef CONFIG_ASSERT
*(._gpio_driver_api.static.gpio_b87_api_)
#endif
*(.text.k_uptime_get) *(.text.sdb) *(.text.device_is_ready)
*(.text.activity_work_handler) *(.text.z_tc32_switch) *(.text.k_work_reschedule)
*(.text.cidoo_battery_read) *(.text.ble_conn_kick) *(.text.tlsr_radio_p24_ack_irq)
*(.text.work_handler) *(.text.__muldi3) *(.text.ble_conn_rx_peek)
*(.text.z_impl_k_busy_wait) *(.text.p24_run) *(.text.activity_event_listener)
*(.text.answered) *(.text.usjis_listener) *(.text.zmk_event_manager_raise)
*(.text.q_put) *(.text.tlsr_radio_set_power_level) *(.text.tlsr_radio_p24_send_listen)
*(.text.exchange) *(.text.ble_profile_active) *(.text.z_sched_wait)
*(.text.tlsr_radio_tx_fifo_rptr) *(.text.z_impl_device_is_ready) *(.text.z_sched_yield)
*(.text.tlsr_radio_set_sn_nesn) *(.text.__aeabi_uidivmod) *(.text.arch_cpu_idle)
*(.text.rx_packet) *(.text.tlsr_radio_clear_irq) *(.text.tlsr_radio_stop)
*(.text.schedule) *(.text.arm) *(.text.z_sched_unready_locked)
*(.text.z_sched_add_to_waitq_locked) *(.text.tc32_rng_add_event) *(.text.band_bits)
*(.text.ble_radio_give) *(.text.zmk_usb_is_hid_ready) *(.text.u32_nonzero)
*(.text.ev_timer_fn) *(.text.crc16) *(.text.k_sem_give)
*(.text.kscan_matrix_schedule) *(.text.fallback_poll) *(.text.reg_set)
*(.text.ble_running) *(.text.gpio_pin_get_dt) *(.text.ble_att_battery_level)
*(.text.pins_take) *(.text.z_timeout_inflight_superseded) *(.text.zmk_physical_layout_kscan_callback)
*(.text.usb_dc_b87_poll) *(.text.zmk_behavior_invoke_binding) *(.text.tlsr827x_irq_mask_read)
*(.text.sys_clock_cycle_get_32) *(.text.memcpy) *(.text.b87_stimer_irq)
*(.text.ble_conn_active) *(.text.z_work_submit_to_queue) *(.text.k_mutex_unlock)
*(.text.zmk_matrix_transform_row_column_to_position) *(.rodata.gpio_b87_config_3) *(.text.tlsr_radio_p24_access)
*(.text.z_sched_wake_thread_locked) *(.text.widening) *(.text.spi_octet)
*(.text.chips_setup) *(.text.low_power) *(.text.z_impl_k_mutex_unlock)
*(.text.pins_up) *(.text.map_layer_id_to_index) *(.text.next_channel)
*(.text.ble_att_ready) *(.text.chip_fill) *(.text.usb_dc_ep_write)
*(.text.event_end) *(.text.tx_acks) *(.text.tlsr_radio_set_rx_dma)
*(.text.zmk_event_manager_handle_from) *(.text.k_work_schedule) *(.text.ble_link_encrypted)
*(.text.ble_battery_poll) *(.text.tlsr_radio_now) *(.text.tlsr_radio_irq_status)
*(.text.tlsr_radio_tx_fifo_restore) *(.text.cidoo_battery_power_in) *(.text.tlsr_radio_p24_settle)
*(.text.tc32_aes128_encrypt) *(.text.sys_slist_remove) *(.text.k_work_schedule_for_queue)
*(.text.event_start) *(.text.switch_poll) *(.text.k_work_submit)
*(.text.k_mutex_lock) *(.text.pins_give) *(.text.tc32_rng_add_sample)
*(.text.ble_thread) *(.text.zmk_physical_layouts_kscan_process_msgq) *(.text.feeder)
*(.text.z_impl_behavior_get_binding) *(.text.notify_queue_locked) *(.text.unpend_thread_no_timeout)
*(.text.pend_locked) *(.text.zmk_keymap_apply_position_state) *(.text.v75pro_backlight_hide)
*(.text.k_msgq_get) *(.text.step) *(.text.ble_link_tx)
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
