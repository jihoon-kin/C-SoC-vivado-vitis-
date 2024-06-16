/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xintc.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xuartlite.h"

//BTN
#define BTN_ID XPAR_AXI_GPIO_0_DEVICE_ID
#define BTN_VEC_ID XPAR_INTC_0_GPIO_0_VEC_ID
#define BTN_CHANNEL 1

//UART
#define UART_ID XPAR_AXI_UARTLITE_0_DEVICE_ID
#define UART_BT_ID XPAR_UARTLITE_1_DEVICE_ID

//INTC
#define INTC_ID XPAR_INTC_0_DEVICE_ID

//JOYSTICK
#define JOYSTICK_ADDR XPAR_MYIP_JOYSTICK_0_S00_AXI_BASEADDR

//LCD
#define IIC_ID XPAR_AXI_IIC_0_DEVICE_ID

//UltraSonic
#define ULT_ADDR XPAR_MYIP_ULTRASONIC_0_S00_AXI_BASEADDR		//ultrasonic address

//PWM 4개
#define PWM_ADDR0 XPAR_MYIP_PWM_0_S00_AXI_BASEADDR
#define PWM_ADDR1 XPAR_MYIP_PWM_1_S00_AXI_BASEADDR
#define PWM_ADDR2 XPAR_MYIP_PWM_2_S00_AXI_BASEADDR
#define PWM_ADDR3 XPAR_MYIP_PWM_3_S00_AXI_BASEADDR

//함수 선언
void BTN_ISR(void *CallBackRef);
void Iic_LCD_write_byte(u8 tx_data, u8 rs);
void Iic_LCD_init(void);
void Iic_movecursor(u8 row, u8 col);
void LCD_write_string(char *string);
void SendHandler(void *CallBackRef, unsigned int EventData);
void RecvHandler(void *CallBackRef, unsigned int EventData);

XGpio btn_device;
XIntc intc;
XUartLite uart_device;
XIic iic_device;

//전역변수 선언
volatile unsigned int *ultrasonic;
volatile unsigned int *joystick;
volatile unsigned int *pwm0;
volatile unsigned int *pwm1;
volatile unsigned int *pwm2;
volatile unsigned int *pwm3;

volatile char Tx[5] = {};
volatile char Rx[5] = {0};
volatile char Tx_btn[5] = {};


char btn_int_flag;

#define xdata joystick[0]
#define ydata joystick[1]

#define BL 3
#define EN 2
#define RW 1
#define RS 0

#define COMMAND 0
#define DATA 1

int main()
{
	XGpio_Config *cfg_ptr;
	ultrasonic = (volatile unsigned int *)ULT_ADDR;
	joystick = (volatile unsigned int*)JOYSTICK_ADDR;
	pwm0 = (volatile unsigned int*)PWM_ADDR0;
	pwm1 = (volatile unsigned int*)PWM_ADDR1;
	pwm2 = (volatile unsigned int*)PWM_ADDR2;
	pwm3 = (volatile unsigned int*)PWM_ADDR3;

    init_platform();

    u32 distance = 0;

    XUartLite_Initialize(&uart_device, UART_BT_ID);

    //GPIO 초기화
    cfg_ptr = XGpio_LookupConfig(BTN_ID);
	XGpio_CfgInitialize(&btn_device, cfg_ptr, cfg_ptr->BaseAddress);
	XGpio_SetDataDirection(&btn_device, BTN_CHANNEL, 0b1111);

	XIntc_Initialize(&intc, INTC_ID);
	XIntc_Connect(&intc, BTN_VEC_ID, (XInterruptHandler)BTN_ISR, (void *)&btn_device);

	XIntc_Enable(&intc, BTN_VEC_ID);
	XIntc_Start(&intc, XIN_REAL_MODE);

	XGpio_InterruptEnable(&btn_device, BTN_CHANNEL); //개별
	XGpio_InterruptGlobalEnable(&btn_device);		//글로벌

	Xil_ExceptionInit(); // Microblaze enable 해준거 설정
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XIntc_InterruptHandler, &intc);
	Xil_ExceptionEnable();

	XUartLite_SetSendHandler(&uart_device, SendHandler, &uart_device);
	XUartLite_SetRecvHandler(&uart_device, RecvHandler, &uart_device);
	XUartLite_EnableInterrupt(&uart_device);

	XIic_Initialize(&iic_device, IIC_ID);
	Iic_LCD_init();

	LCD_write_string("distance :   cm");
	Iic_movecursor(1, 0);

	//pwm_period
	pwm0[1] = pwm1[1] = pwm2[1] = pwm3[1]= 200000;

    while(1){


    	u32 distance = ultrasonic[0];


//    	pwm0[0] = pwm1[0] = pwm2[0] = pwm3[0]= 200000;
       	//if문 전에 미리 pwm값 저장
        int original_pwm0 = pwm0[0];
        int original_pwm1 = pwm1[0];
        int original_pwm2 = pwm2[0];
        int original_pwm3 = pwm3[0];


        sprintf(Tx, "%d\n\r", distance);
        xil_printf("%d\n\r",distance);
        SendHandler(&uart_device, Tx);

        //RecvHandler
		if(Rx){
			RecvHandler(&uart_device, Rx);
		}

		//조이스틱 receive
		int joystickreceivedValue;
		memcpy((void *)&joystickreceivedValue, (void *)Rx, sizeof(joystickreceivedValue));
		joystickreceivedValue = atoi(Rx);

		xil_printf("Rx data : %d\n\r", joystickreceivedValue);

        //조이스틱

        if((pwm0[0] = pwm1[0] = pwm2[0] = pwm3[0]) > 60000){
        switch(joystickreceivedValue) {
        	case 1:
        		pwm0[0] = original_pwm0 - 60000;
        		xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
        	case 2:
                pwm0[0] = original_pwm0 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 3:
                pwm0[0] = original_pwm0 - 20000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 4:
                pwm0[0] = original_pwm0 - 60000;
                pwm1[0] = original_pwm1 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 5:
                pwm0[0] = original_pwm0 - 40000;
                pwm1[0] = original_pwm1 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 6:
                pwm0[0] = original_pwm0 - 20000;
                pwm1[0] = original_pwm1 - 20000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 7:
                pwm1[0] = original_pwm1 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 8:
                pwm1[0] = original_pwm1 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 9:
            	pwm0[0] = original_pwm0 - 20000;
            	xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 10:
                pwm0[0] = original_pwm0 - 60000;
                pwm2[0] = original_pwm2 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 11:
                pwm0[0] = original_pwm0 - 40000;
                pwm2[0] = original_pwm2 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 12:
                pwm0[0] = original_pwm0 - 20000;
                pwm2[0] = original_pwm2 - 20000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 13:
                pwm1[0] = original_pwm1 - 60000;
                pwm3[0] = original_pwm3 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 14:
                pwm1[0] = original_pwm1 - 40000;
                pwm3[0] = original_pwm3 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 15:
                pwm1[0] = original_pwm1 - 20000;
                pwm3[0] = original_pwm3 - 20000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 16:
                pwm2[0] = original_pwm2 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 17:
                pwm2[0] = original_pwm2 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 18:
                pwm2[0] = original_pwm2 - 20000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 19:
                pwm2[0] = original_pwm2 - 60000;
                pwm3[0] = original_pwm3 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 20:
                pwm2[0] = original_pwm2 - 40000;
                pwm3[0] = original_pwm3 - 40000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 21:
                pwm2[0] = original_pwm2 - 20000;
                pwm3[0] = original_pwm3 - 20000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 22:
                pwm3[0] = original_pwm3 - 60000;
                xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
                break;
            case 23:
               pwm3[0] = original_pwm3 - 40000;
               xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
               break;
            case 24:
               pwm3[0] = original_pwm3 - 20000;
               xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
               break;
            default:
               // Neutral zone
                break;
        }
        }
        //while문 탈출하면 다시 초기화(버튼 상/하 pwm)

        pwm0[0] = original_pwm0;
        pwm1[0] = original_pwm1;
        pwm2[0] = original_pwm2;
        pwm3[0] = original_pwm3;

    	//BTN 상/하

    	xil_printf("1:%d\n\r 2:%d\n\r 3:%d\n\r 4:%d\n\r ", pwm0[0], pwm1[0], pwm2[0], pwm3[0]);
    	char btn_value;
    	memcpy((void *)&btn_value, (void *)Rx, sizeof(btn_value));

    	switch(btn_value) {
    		case 'a':	//상
    			if(pwm0[0] < 200000){
    				pwm0[0] += 10000;
    				pwm1[0] += 10000;
    	           	pwm2[0] += 10000;
    	            pwm3[0] += 10000;
    			}else{
    	            pwm0[0] = pwm1[0] = pwm2[0] = pwm3[0]= 200000;
    			}
    	        break;
    	    case 'b':	//하
    	    	if(pwm0[0] > 0){
    	    		pwm0[0] -= 10000;
    	    	    pwm1[0] -= 10000;
    	    	    pwm2[0] -= 10000;
    	    	    pwm3[0] -= 10000;
    	    	}else{
    	    		pwm0[0] = pwm1[0] = pwm2[0] = pwm3[0]= 0;
    	    	}
    	    	break;
    	}
    }
    cleanup_platform();
    return 0;
}
void BTN_ISR(void *CallBackRef){
	//XGpio *Gpio_ptr = (XGpio *)CallBackRef;
	btn_int_flag =1;
	XGpio_InterruptClear(&btn_device, BTN_CHANNEL);
	XGpio_InterruptDisable(&btn_device, BTN_CHANNEL);
	return;
}
void Iic_LCD_write_byte(u8 tx_data, u8 rs){ //d7 d6 d5 d4 BL EN RW RS
	u8 data_t[4] = {0,};
	data_t[0] = (tx_data & 0xf0) | (1 << BL) | (rs & 1) | (1 << EN);	//상위 en = 1
	data_t[1] = (tx_data & 0xf0) | (1 << BL) | (rs & 1); //상위 en = 0
	data_t[2] = (tx_data << 4) | (1 << BL) | (rs & 1) | (1 << EN); //하위 en = 1
	data_t[3] = (tx_data << 4) | (1 << BL) | (rs & 1); //하위 en = 0
	XIic_Send(iic_device.BaseAddress, 0x27, &data_t, 4, XIIC_STOP);
}
void Iic_LCD_init(void){
	MB_Sleep(15);
	Iic_LCD_write_byte(0x33, COMMAND);
	Iic_LCD_write_byte(0x32, COMMAND);
	Iic_LCD_write_byte(0x28, COMMAND);
	Iic_LCD_write_byte(0x0c, COMMAND);
	Iic_LCD_write_byte(0x01, COMMAND);
	Iic_LCD_write_byte(0x06, COMMAND);
	MB_Sleep(10);
	return;
}
void Iic_movecursor(u8 row, u8 col){
	row %= 2;
	col %= 40;
	Iic_LCD_write_byte(0x80 | (row << 6) | col, COMMAND);
	return;
}
void LCD_write_string(char *string){
	for (int i = 0; string[i]; i++){
		Iic_LCD_write_byte(string[i], DATA);
	}
	return;
}
void SendHandler(void *CallBackRef, unsigned int EventData){
	XUartLite_Send(&uart_device, (u8 *)Tx, sizeof(Tx));
	XUartLite_Send(&uart_device, (u8 *)Tx_btn, sizeof(Tx_btn));
	return;
}
void RecvHandler(void *CallBackRef, unsigned int EventData){
	XUartLite_Recv(&uart_device, (u8 *)Rx, sizeof(Rx));
	return;
}

