# Betaflight sources for cmake


set(BETAFLIGHT_SOURCES
    "build/build_config.c"
    "build/debug.c"
    "build/version.c"
#    "main.c"
    "cli/cli.c"
    "cli/settings.c"
#    "drivers/adc.c"
    "drivers/buf_writer.c"
    "drivers/bus.c"
#    "drivers/bus_i2c_config.c"
    "drivers/bus_i2c_busdev.c"
    "drivers/bus_i2c_soft.c"
#    "drivers/bus_spi.c"
#    "drivers/bus_spi_config.c"
#    "drivers/bus_spi_pinconfig.c"
    "drivers/buttons.c"
    "drivers/display.c"
    "drivers/dma_reqmap.c"
    "drivers/exti.c"
    "drivers/io.c"
    "drivers/light_led.c"
    "drivers/mco.c"
    "drivers/pinio.c"
    "drivers/resource.c"
#    "drivers/rcc.c"
    "drivers/serial.c"
#    "drivers/serial_pinconfig.c"
#    "drivers/serial_uart.c"
#    "drivers/serial_uart_pinconfig.c"
    "drivers/sound_beeper.c"
    "drivers/stack_check.c"
#    "drivers/system.c"
    "drivers/timer_common.c"
#    "drivers/timer.c"
    "drivers/transponder_ir_arcitimer.c"
    "drivers/transponder_ir_ilap.c"
    "drivers/transponder_ir_erlt.c"
    "fc/board_info.c"
    "fc/config.c"
    "fc/dispatch.c"
    "fc/hardfaults.c"
    "fc/tasks.c"
    "fc/runtime_config.c"
    "fc/stats.c"
    "io/beeper.c"
    "io/piniobox.c"
    "io/serial.c"
    "io/smartaudio_protocol.c"
    "io/statusindicator.c"
    "io/tramp_protocol.c"
    "io/transponder_ir.c"
    "io/usb_cdc_hid.c"
    "io/usb_msc.c"
    "msp/msp.c"
    "msp/msp_box.c"
    "msp/msp_serial.c"
    "scheduler/scheduler.c"
    "sensors/adcinternal.c"
    "sensors/battery.c"
    "sensors/current.c"
    "sensors/voltage.c"
    "target/config_helper.c"
    "fc/init.c"
    "fc/controlrate_profile.c"
    "drivers/camera_control.c"
    "drivers/accgyro/gyro_sync.c"
    "drivers/pwm_esc_detect.c"
#    "drivers/pwm_output.c"
    "drivers/rx/rx_spi.c"
#    "drivers/rx/rx_xn297.c"
    "drivers/rx/rx_pwm.c"
    "drivers/serial_softserial.c"
    "fc/core.c"
    "fc/rc.c"
    "fc/rc_adjustments.c"
    "fc/rc_controls.c"
    "fc/rc_modes.c"
    "flight/position.c"
    "flight/failsafe.c"
    "flight/gps_rescue.c"
    "flight/imu.c"
    "flight/mixer.c"
    "flight/mixer_tricopter.c"
    "flight/pid.c"
    "flight/servos.c"
    "flight/servos_tricopter.c"
    "io/serial_4way.c"
    "io/serial_4way_avrootloader.c"
    "io/serial_4way_stk500v2.c"
    "rx/ibus.c"
    "rx/jetiexbus.c"
    "rx/msp.c"
    "rx/pwm.c"
    "rx/rx.c"
    "rx/rx_spi.c"
    "rx/rx_spi_common.c"
    "rx/crsf.c"
    "rx/sbus.c"
    "rx/sbus_channels.c"
    "rx/spektrum.c"
    "io/spektrum_vtx_control.c"
    "io/spektrum_rssi.c"
    "rx/sumd.c"
    "rx/sumh.c"
    "rx/xbus.c"
    "rx/fport.c"
    "sensors/acceleration.c"
    "sensors/boardalignment.c"
    "sensors/compass.c"
    "sensors/gyro.c"
    "flight/gyroanalyse.c"
    "flight/rpm_filter.c"
    "sensors/initialisation.c"
    "blackbox/blackbox.c"
    "blackbox/blackbox_encoding.c"
#    "blackbox/blackbox_io.c"
    "cms/cms.c"
    "cms/cms_menu_blackbox.c"
    "cms/cms_menu_builtin.c"
    "cms/cms_menu_failsafe.c"
    "cms/cms_menu_gps_rescue.c"
    "cms/cms_menu_imu.c"
    "cms/cms_menu_ledstrip.c"
    "cms/cms_menu_misc.c"
    "cms/cms_menu_osd.c"
    "cms/cms_menu_power.c"
    "cms/cms_menu_saveexit.c"
    "cms/cms_menu_vtx_rtc6705.c"
    "cms/cms_menu_vtx_smartaudio.c"
    "cms/cms_menu_vtx_tramp.c"
#    "drivers/display_ug2864hsweg01.c"
    "drivers/light_ws2811strip.c"
    "drivers/rangefinder/rangefinder_hcsr04.c"
    "drivers/rangefinder/rangefinder_lidartf.c"
#    "drivers/serial_escserial.c"
    "drivers/vtx_common.c"
    "drivers/vtx_table.c"
    "io/dashboard.c"
#    "io/displayport_max7456.c"
    "io/displayport_msp.c"
#    "io/displayport_oled.c"
    "io/displayport_srxl.c"
    "io/displayport_crsf.c"
    "io/displayport_hott.c"
    "io/rcdevice_cam.c"
    "io/rcdevice.c"
    "io/gps.c"
    "io/ledstrip.c"
#    "io/pidaudio.c"
    "osd/osd.c"
    "osd/osd_elements.c"
    "sensors/barometer.c"
    "sensors/rangefinder.c"
    "telemetry/telemetry.c"
#    "telemetry/crsf.c"
#    "telemetry/srxl.c"
    "telemetry/frsky_hub.c"
    "telemetry/hott.c"
    "telemetry/jetiexbus.c"
    "telemetry/smartport.c"
    "telemetry/ltm.c"
    "telemetry/mavlink.c"
    "telemetry/msp_shared.c"
    "telemetry/ibus.c"
    "telemetry/ibus_shared.c"
    "sensors/esc_sensor.c"
    "io/vtx_string.c"
    "io/vtx.c"
    "io/vtx_rtc6705.c"
    "io/vtx_smartaudio.c"
    "io/vtx_tramp.c"
    "io/vtx_control.c"

    # SITL:
    "drivers/accgyro/accgyro_fake.c"
    "drivers/barometer/barometer_fake.c"
    "drivers/compass/compass_fake.c")

list(TRANSFORM BETAFLIGHT_SOURCES PREPEND 
    "${CMAKE_CURRENT_SOURCE_DIR}/betaflight/src/main/")

set(BETAFLIGHT_SOURCES ${BETAFLIGHT_SOURCES} 
    "${CMAKE_CURRENT_SOURCE_DIR}/betaflight/lib/main/dyad/dyad.c")

file(GLOB BETAFLIGHT_G_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/betaflight/src/main/pg/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/betaflight/src/main/common/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/betaflight/src/main/config/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/drivers/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/io/*.c")

list(REMOVE_ITEM BETAFLIGHT_G_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/betaflight/src/main/common/string_light.c")

set(BETAFLIGHT_SOURCES 
    ${BETAFLIGHT_SOURCES} ${BETAFLIGHT_G_SOURCES}
    PARENT_SCOPE)
