/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include <stdio.h>
#include <math.h>
#define DELAY (600000)  
volatile uint8_t game_started = 0;
volatile uint8_t launch_ball = 0;
#define MAX_BALLS 20 // 





void UART_SendByte(UART_Regs *uart, uint8_t data) {
    DL_UART_Main_transmitDataBlocking(uart, data);
}

void UART_SendString(UART_Regs *uart, char *str) {
    while(*str != '\0') {
        UART_SendByte(uart, *str);
        str++;
    }
}

void SendToScreen(char *cmd) {
    UART_SendString(UART0, cmd);
    UART_SendByte(UART0, 0xFF);
    UART_SendByte(UART0, 0xFF);
    UART_SendByte(UART0, 0xFF);
}





typedef struct {
    int center_x;   // ???? X
    int center_y;   // ???? Y
    int r;          // ????
    int w;          // ???
    int h;          // ???
    int pic_id;     // ??ID
    float speed;    // ??
    
    // ?????? static ???????
    float angle;    
    int old_x;      
    int old_y;      
} GameObject;

	GameObject balls[MAX_BALLS];

void Rotate_Image(GameObject *obj)
{
   char buf[64];

    // ??????(? obj-> ??????????)
    if (obj->old_x == -1) {
        obj->old_x = obj->center_x + (int)(obj->r * cos(obj->angle));
        obj->old_y = obj->center_y + (int)(obj->r * sin(obj->angle));
    }

    // ???????????
    obj->angle += obj->speed;
    if (obj->angle > 2 * 3.1415926) { obj->angle -= 2 * 3.1415926; }

    int new_x = obj->center_x + (int)(obj->r * cos(obj->angle));
    int new_y = obj->center_y + (int)(obj->r * sin(obj->angle));

    // ??????
    if (new_x != obj->old_x || new_y != obj->old_y) {
        sprintf(buf, "fill %d,%d,%d,%d,0", obj->old_x, obj->old_y, obj->w, obj->h);
        SendToScreen(buf);
        
        sprintf(buf, "pic %d,%d,%d", new_x, new_y, obj->pic_id);
        SendToScreen(buf);
        
        obj->old_x = new_x; 
        obj->old_y = new_y;
    }
}
void launch_forward(){
 int old_x = -1; 
 int old_y = -1;
    char buf[64];
int x0 = 119;
int y0 = 382;
    // ?????????
int  step = 8;
        old_x = x0;
        old_y = y0;
    // 2. ???????
    int new_x = x0 ;
    int new_y = y0 -1;

    while (1) {
        delay_cycles(DELAY);
			new_y-=step;

    // 3. ??????,???????
    if ( new_y != old_y) {
        // ? ????:????(0)?????
        sprintf(buf, "fill %d,%d,%d,%d,0", old_x, old_y, 34, 34);
        SendToScreen(buf);
        
        // ? ????:??????? 3D ?
        sprintf(buf, "pic %d,%d,%d", new_x, new_y, 2);
        SendToScreen(buf);
        
        // ????
        old_x = new_x; 
        old_y = new_y;
    }
		if(old_y <= 274){
		  return;
		}
}}

// ====================== ??? ======================
int main(void)
{
    SYSCFG_DL_init();
    __enable_irq();
    // ?? UART0 ?????(??? SysConfig ???? RX Interrupt ????)
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
	
int active_balls = 0;
		
  
    /* ====== ????:?????? ====== */
    while (game_started == 0) {
        // ?????????????,?????? 0x5A ??
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        delay_cycles(4000000); 
    }

    /* ====== ????:????,???????? ====== */
    // ?????????????
    SendToScreen("t0.txt=\"Score: 0\"");
	
while(1){
	delay_cycles(DELAY);
		if(launch_ball == 1){
			// ????:?????????????
            if (active_balls < MAX_BALLS) {
                
                launch_forward(); // ????
                
                // ?? ????:??????????,?????????!
                balls[active_balls] = (GameObject){120, 168, 107, 34, 34, 2, 0.05, 1.5708, -1, -1};
               
                active_balls++;   // ??????? +1
            }
            
            launch_ball = 0;
		}
		for(int i=0;i<active_balls;i++){
						 Rotate_Image(&balls[i]);
						}
       
     
	}
}

/* ====== ????:??????????? ====== */
void UART_0_INST_IRQHandler(void) {
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_RX: // ??????
{
            // ?? ????:?????????!
            uint8_t rx_data = DL_UART_Main_receiveData(UART_0_INST);
            
            if (rx_data == 0x5A) {
                game_started = 1; // ?? 5A,????
            }
            else if(rx_data == 0x4A) {
                launch_ball = 1;  // ?? 4A,????
            }
            break;
        }
        default: break;
    }
	}