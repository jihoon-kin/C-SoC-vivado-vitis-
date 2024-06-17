//Copyright 1986-2019 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2019.2 (win64) Build 2708876 Wed Nov  6 21:40:23 MST 2019
//Date        : Mon Jun  3 08:55:39 2024
//Host        : Digital-4 running 64-bit major release  (build 9200)
//Command     : generate_target drone_wrapper.bd
//Design      : drone_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module drone_wrapper
   (echo_0,
    iic_rtl_scl_io,
    iic_rtl_sda_io,
    push_buttons_4bits_tri_i,
    pwm_512_0,
    pwm_512_1,
    pwm_512_2,
    pwm_512_3,
    reset,
    sys_clock,
    trigger_0,
    usb_uart_0_rxd,
    usb_uart_0_txd,
    usb_uart_rxd,
    usb_uart_txd,
    vauxn15_0,
    vauxn6_0,
    vauxp15_0,
    vauxp6_0);
  input echo_0;
  inout iic_rtl_scl_io;
  inout iic_rtl_sda_io;
  input [3:0]push_buttons_4bits_tri_i;
  output pwm_512_0;
  output pwm_512_1;
  output pwm_512_2;
  output pwm_512_3;
  input reset;
  input sys_clock;
  output trigger_0;
  input usb_uart_0_rxd;
  output usb_uart_0_txd;
  input usb_uart_rxd;
  output usb_uart_txd;
  input vauxn15_0;
  input vauxn6_0;
  input vauxp15_0;
  input vauxp6_0;

  wire echo_0;
  wire iic_rtl_scl_i;
  wire iic_rtl_scl_io;
  wire iic_rtl_scl_o;
  wire iic_rtl_scl_t;
  wire iic_rtl_sda_i;
  wire iic_rtl_sda_io;
  wire iic_rtl_sda_o;
  wire iic_rtl_sda_t;
  wire [3:0]push_buttons_4bits_tri_i;
  wire pwm_512_0;
  wire pwm_512_1;
  wire pwm_512_2;
  wire pwm_512_3;
  wire reset;
  wire sys_clock;
  wire trigger_0;
  wire usb_uart_0_rxd;
  wire usb_uart_0_txd;
  wire usb_uart_rxd;
  wire usb_uart_txd;
  wire vauxn15_0;
  wire vauxn6_0;
  wire vauxp15_0;
  wire vauxp6_0;

  drone drone_i
       (.echo_0(echo_0),
        .iic_rtl_scl_i(iic_rtl_scl_i),
        .iic_rtl_scl_o(iic_rtl_scl_o),
        .iic_rtl_scl_t(iic_rtl_scl_t),
        .iic_rtl_sda_i(iic_rtl_sda_i),
        .iic_rtl_sda_o(iic_rtl_sda_o),
        .iic_rtl_sda_t(iic_rtl_sda_t),
        .push_buttons_4bits_tri_i(push_buttons_4bits_tri_i),
        .pwm_512_0(pwm_512_0),
        .pwm_512_1(pwm_512_1),
        .pwm_512_2(pwm_512_2),
        .pwm_512_3(pwm_512_3),
        .reset(reset),
        .sys_clock(sys_clock),
        .trigger_0(trigger_0),
        .usb_uart_0_rxd(usb_uart_0_rxd),
        .usb_uart_0_txd(usb_uart_0_txd),
        .usb_uart_rxd(usb_uart_rxd),
        .usb_uart_txd(usb_uart_txd),
        .vauxn15_0(vauxn15_0),
        .vauxn6_0(vauxn6_0),
        .vauxp15_0(vauxp15_0),
        .vauxp6_0(vauxp6_0));
  IOBUF iic_rtl_scl_iobuf
       (.I(iic_rtl_scl_o),
        .IO(iic_rtl_scl_io),
        .O(iic_rtl_scl_i),
        .T(iic_rtl_scl_t));
  IOBUF iic_rtl_sda_iobuf
       (.I(iic_rtl_sda_o),
        .IO(iic_rtl_sda_io),
        .O(iic_rtl_sda_i),
        .T(iic_rtl_sda_t));
endmodule
