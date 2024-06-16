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
#include "xil_exception.h"
#include "xuartlite.h"

#define BTN_ID XPAR_AXI_GPIO_0_DEVICE_ID
#define UART_ID XPAR_AXI_UARTLITE_0_DEVICE_ID
#define INTC_ID XPAR_AXI_INTC_0_DEVICE_ID
#define BTN_VEC_ID XPAR_INTC_0_GPIO_0_VEC_ID
#define UART_VEC_ID XPAR_INTC_0_UARTLITE_0_VEC_ID

#define BTN_CHANNEL 1

void BTN_ISR(void *CallBackRef); // 프로트타입 함수
void SendHandler(void *CallBackRef, unsigned int EventData);
void RecvHandler(void *CallBackRef, unsigned int EventData);

XGpio btn_device;
XIntc intc;
XUartLite uart_device;

int main()
{
    XGpio_Config *cfg_ptr;
	init_platform();

    print("Start!!\n\r");

    XUartLite_Initialize(&uart_device, UART_ID); // uart 초기화

    cfg_ptr = XGpio_LookupConfig(BTN_ID); // Gpio 초기화 설정
	XGpio_CfgInitialize(&btn_device, cfg_ptr, cfg_ptr->BaseAddress);
	XGpio_SetDataDirection(&btn_device, BTN_CHANNEL, 0b1111);

	XIntc_Initialize(&intc, INTC_ID); // 인터럽트 컨트롤러 초기화
	// XIntc_Connect(인터럽트 주소, 인터럽트 벡터의 아이디, (타입)인터럽트를 처리하는 함수, (타입)어떤 디바이스에서 인터럽트 발생했는지)
	XIntc_Connect(&intc, UART_VEC_ID, (XInterruptHandler)XUartLite_InterruptHandler, (void *)&uart_device);

	XUartLite_SetRecvHandler(&uart_device, RecvHandler, &uart_device); // 수신완료 인터럽트
	XUartLite_SetSendHandler(&uart_device, SendHandler, &uart_device); // 송신완료 인터럽트
	XUartLite_EnableInterrupt(&uart_device);

	XIntc_Connect(&intc, BTN_VEC_ID, (XInterruptHandler)BTN_ISR, (void *)&btn_device);

	XIntc_Enable(&intc, BTN_VEC_ID); // 활성화
	XIntc_Enable(&intc, UART_VEC_ID);
	XIntc_Start(&intc, XIN_REAL_MODE); // 스타트

	XGpio_InterruptEnable(&btn_device, BTN_CHANNEL); // gpio 인터럽트 설정
	XGpio_InterruptGlobalEnable(&btn_device); // gpio가 여러개 있을때 해줘야함.

	Xil_ExceptionInit(); // Microblaze enable 해준거 설정
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)XIntc_InterruptHandler, &intc);
	Xil_ExceptionEnable();

	while(1){

	}

    cleanup_platform();
    return 0;
}

void BTN_ISR(void *CallBackRef){
	XGpio *Gpio_ptr = (XGpio *)CallBackRef;
	print("btn interrupt \n\r");
	if(XGpio_DiscreteRead(Gpio_ptr, BTN_CHANNEL) & 0b0010){ // 0b0010 : left 눌렀을때만 1이 된다.
		print("btn left pushed\n\r");
	}

	XGpio_InterruptClear(Gpio_ptr, BTN_CHANNEL);
	return;
}

void SendHandler(void *CallBackRef, unsigned int EventData){
	return;
}
void RecvHandler(void *CallBackRef, unsigned int EventData){
	u8 rxData;
	XUartLite_Recv(&uart_device, &rxData, 1); // 한바이트씩 받자, 수신받은걸 rxData에 저장한다.
	xil_printf("recv %c\n\r", rxData);
	return;
}
































