/*
 * The code and constants the V75 Pro reads most often, at the start of .text (TEXT_SECTIONS, CMakeLists.txt): the
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

*(.text.sdb) *(.text.schedule) *(.text.tlsr_radio_tx_fifo_restore)
*(.text.u32_nonzero) *(.text.tlsr_radio_set_ble_channel) *(.text.tlsr_radio_p24_settle)
*(.text.k_mutex_unlock) *(.text.k_msgq_get) *(.text.z_sched_unready_locked)
*(.text.z_impl_k_mutex_lock) *(.text.next_timeout) *(.text.sys_clock_set_timeout)
*(.text.b87_stimer_program_compare) *(.text.gpio_pin_set_dt) *(.text.gpio_b87_pin_configure)
*(.text.in_done) *(.text.schedule_for_queue_locked) *(.text.z_add_timeout)
*(.text.__udivsi3) *(.text.k_work_reschedule) *(.text.gpio_b87_pull)
*(.text.kscan_matrix_read_end) *(.text.zmk_event_manager_handle_from) *(.text.tlsr_radio_set_rx_dma)
*(.text.z_swap_locked) *(.text.z_timeout_inflight_superseded) *(.text.device_is_ready)
*(.text.ble_link_encrypted) *(.text.sys_clock_tick_get) *(.text.elapsed)
*(.text.work_timeout) *(.text.work_queue_main) *(.text.z_swap)
*(.rodata.gpio_b87_config_0) *(.text.submit_to_queue_locked) *(.text.notify_queue_locked)
*(.text.z_sched_ready_locked) *(.text.sys_slist_remove) *(.text.z_impl_behavior_get_binding)
*(.text.band_bits) *(.text.__muldi3) *(.text.z_pend_curr)
*(.text.k_work_reschedule_for_queue) *(.text.__aeabi_uidivmod) *(.text.z_sched_wait)
*(.text.gpio_b87_port_clear_bits_raw) *(.text.tlsr827x_irq_mask_read) *(.text.reprogram_next)
*(.text.sys_clock_announce_locked) *(.text.p24_rf_isr) *(.text.u32_bit0_set)
*(.text.tlsr_radio_conn_event_prepare) *(.text.requests_pending) *(.text.ccm_ctr)
*(.text.low_power) *(.text.switch_poll) *(.text.p24_run)
*(.text.k_uptime_get_32) *(.text.ble_conn_wait_ticks) *(.text.z_timer_expiration_handler)
*(.text.arm) *(.text.widening) *(.text.kscan_gpio_pin_get)
*(.rodata.gpio_b87_config_2) *(.rodata.gpio_b87_config_1) *(.text.gpio_b87_port_get_raw)
*(.text.pend_locked) *(.text.usb_dc_b87_poll) *(.rodata.kscan_matrix_config_0)
*(.text.tlsr_radio_set_sn_nesn) *(.text.idle) *(.text.usb_poll_timer_isr)
*(.text.next_channel) *(.text.as_zmk_position_state_changed) *(.text.tlsr_radio_start_conn_event)
*(.text.ble_conn_rx_peek) *(.text.z_sched_wake_thread_locked) *(.text.gpio_b87_port_set_bits_raw)
*(.text.kscan_matrix_work_handler) *(.text.k_uptime_get) *(.text.z_thread_timeout)
*(.text.chip_fill) *(.text.reset_isr) *(.text.event_end)
*(.text.chip_end) *(.text.z_impl_k_busy_wait) *(.text.ble_conn_kick)
*(.text.work_handler) *(.text.tlsr827x_analog_write) *(.text.kscan_matrix_schedule)
*(.text.z_tc32_mul32x32) *(.text.read_mode_switch) *(.text.tlsr_radio_p24_access)
*(.text.q_put) *(.text.z_get_next_switch_handle) *(.text.sys_clock_elapsed)
*(.text.ticks_now) *(.text.zmk_debounce_get_changed) *(.text.zmk_debounce_is_active)
*(.rodata.gpio_b87_config_3)
#ifndef CONFIG_ASSERT
*(._gpio_driver_api.static.gpio_b87_api_)
#endif
*(.text.spi_octet) *(.text.ble_running) *(.text.check)
*(.text.k_work_schedule_for_queue) *(.text.tlsr_radio_off) *(.text.arch_cpu_idle)
*(.text.z_tc32_switch) *(.text.z_sched_add_to_waitq_locked) *(.text.ble_att_ready)
*(.text.tlsr_radio_sn_nesn) *(.text.tlsr_radio_tx_fifo_rptr) *(.text.zmk_debounce_is_pressed)
*(.text.tlsr_radio_set_power_level) *(.text.cidoo_battery_read) *(.text.tlsr_radio_stop)
*(.text.ble_hid_leds) *(.text.map_layer_id_to_index) *(.text.tlsr827x_analog_read)
*(.text.gpio_b87_ie_update) *(.text.usb_poll) *(.text.tlsr827x_analog_wait)
*(.text.chip_begin) *(.text.chips_setup) *(.text.ble_radio_take)
*(.text.zmk_event_manager_raise) *(.text.zmk_debounce_update) *(.text.zmk_behavior_invoke_binding)
*(.text.ble_att_battery_level) *(.text.atomic_get) *(.text.ble_radio_give)
*(.text.crc16) *(.text.unpend_thread_no_timeout) *(.text.memset)
*(.text.tlsr_radio_conn_timing) *(.text.tlsr_radio_irq_status) *(.text.p24_running)
*(.text.tlsr_radio_rx_timestamp) *(.text.ccm_mac) *(.text.zmk_usb_is_hid_ready)
*(.text.zmk_physical_layout_kscan_callback) *(.text.fallback_poll) *(.text.rx_packet)
*(.text.answered) *(.text.event_start) *(.text.ep_data_isr)
*(.text.rf_isr) *(.text.z_impl_k_sem_take) *(.text.gpio_port_get)
*(.text.pins_take) *(.text.z_sched_yield) *(.text.z_try_abort_timeout)
*(.text.tlsr_radio_p24_ack_irq) *(.text.unschedule_locked) *(.text.ble_zmk_flush)
*(.text.ble_thread) *(.text.ble_smp_poll) *(.text.tlsr_radio_p24_send_listen)
*(.text.tlsr_radio_clear_irq) *(.text.tlsr_radio_now) *(.text.tlsr_radio_p24_channel)
*(.text.ble_link_authenticated) *(.text.pins_up) *(.text.z_impl_k_sem_give)
*(.text.reg_set) *(.text.tlsr_analog_write) *(.text.ble_usb_host)
*(.text.cidoo_battery_power_in) *(.text.ble_low_power_poll) *(.text.z_impl_atomic_set)
*(.text.usjis_listener) *(.text.z_impl_k_msgq_get) *(.text.tc32_aes128_encrypt)
*(.text.z_tc32_udiv64_avg_mulhi) *(.text.ble_battery_poll) *(.text.zmk_physical_layouts_kscan_process_msgq)
*(.text.queue) *(.text.k_sem_give) *(.text.ble_profile_active)
*(.text.as_zmk_keycode_state_changed) *(.text.pins_give) *(.text.__lshrdi3)
*(.text.zmk_matrix_transform_row_column_to_position) *(.text.v75pro_backlight_hide) *(.text.zmk_endpoint_send_report)
*(.text.sys_clock_cycle_get_32) *(.text.ble_conn_tx) *(.text.ev_timer_fn)
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
