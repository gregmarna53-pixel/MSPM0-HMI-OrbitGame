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
#include <stdlib.h>
#include <string.h>
#define DELAY (600000)  
volatile uint8_t game_started_easy = 0;
volatile uint8_t game_started_mid = 0;
volatile uint8_t game_started_hard = 0;
volatile uint8_t launch_ball = 0;
volatile uint8_t is_collision = 0;
int max_score = 0;
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

void LED_All_Off(void) {
    DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_0_PIN | GPIO_LEDS_PIN_1_PIN | GPIO_LEDS_PIN_2_PIN | GPIO_LEDS_PIN_3_PIN | GPIO_LEDS_PIN_4_PIN );
}
// ????
void LED_Only_Green(void) {
    LED_All_Off();
    DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_0_PIN| GPIO_LEDS_PIN_4_PIN);
	
}
// ????
void LED_Only_Red(void) {
    LED_All_Off();
    DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_0_PIN| GPIO_LEDS_PIN_3_PIN);
}
// ??????(??????)
void LED_Flow_Run(void) {
    static uint8_t flow_index = 0;
    LED_All_Off();
    switch(flow_index) {
        case 0: DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_0_PIN); break;
        case 1: DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_1_PIN); break;
        case 2: DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_2_PIN); break;
        case 3: DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_3_PIN); break;
        case 4: DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_PIN_4_PIN); break;
    }
    flow_index = (flow_index + 1) % 5;
		
}


void Beep_Play1(uint16_t time_ms)
{
    for(uint32_t i=0; i < time_ms * 10; i++){
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_BEEP_PIN); 
        delay_cycles(10000);  // ????,????,??????
    }
    DL_GPIO_clearPins(GPIO_LEDS_PORT, GPIO_LEDS_BEEP_PIN);
}



typedef struct {
    int center_x;   // ???? X
    int center_y;   // ???? Y
    int r;          // ????
    int w;          // ???
    int h;          // ???
    int pic_id;     // ??ID
    float speed;    // ??
    float angle;    
    int old_x;      
    int old_y;      
} GameObject;

	GameObject balls[MAX_BALLS];
// ????????????,??? w ? h ??
// ??????? 1 ???:?? ID
GameObject Create_Ball(int pic_id,float sped) {
    GameObject obj;
    if(pic_id ==3){
				obj.center_x = 113;      // ??????? X
				obj.center_y = 161;      // ??????? Y
				obj.r = 115;   }
    // 1. ????????????
		else{
    obj.center_x = 120;      // ??????? X
    obj.center_y = 168;      // ??????? Y
    obj.r = 107;  
		}			// ???????
    obj.angle = 1.5708;      // ????????? (PI/2,????)
    obj.old_x = -1;          // ??????
    obj.old_y = -1;
	
    obj.speed = sped;
    obj.pic_id = pic_id;

    // 2. ????? pic_id,?????? (??????????)
    switch(pic_id) {
        case 2: // ????
            obj.w = 34;
            obj.h = 34;
           
            break;
            
        case 3: // ?? ID=3 ???“???”,?????
            obj.w = 46;
            obj.h = 46;
         
            break;
            
        default: // ?????
            obj.w = 26;
            obj.h = 26;
            
            break;
    }
    
    return obj;
}
int active_balls = 0;
void Rotate_Image(GameObject *obj)
{
   char buf[64];

  
    if (obj->old_x == -1) {
        obj->old_x = obj->center_x + (int)(obj->r * cos(obj->angle));
        obj->old_y = obj->center_y + (int)(obj->r * sin(obj->angle));
    }

    
    obj->angle += obj->speed;
    if (obj->angle > 2 * 3.1415926) { obj->angle -= 2 * 3.1415926; }

    int new_x = obj->center_x + (int)(obj->r * cos(obj->angle));
    int new_y = obj->center_y + (int)(obj->r * sin(obj->angle));

    
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
int y0 = 365;
int  step = 8;
old_x = x0;
old_y = y0;

    int new_x = x0 ;
    int new_y = y0 -1;

    while (1) {
        delay_cycles(DELAY);
			  new_y-=step;

    // 3. ??????,???????
				if ( new_y != old_y) {
						// ? ????:????(0)?????
						sprintf(buf, "fill %d,%d,%d,%d,0", old_x, old_y, 26, 26);
						SendToScreen(buf);
						
						// ? ????:??????? 3D ?
						sprintf(buf, "pic %d,%d,%d", new_x, new_y, 4);
						SendToScreen(buf);
						
						// ????
						old_x = new_x; 
						old_y = new_y;
				}
				for(int i=0;i<active_balls;i++){
									 Rotate_Image(&balls[i]);
									}
				 
					if(old_y <= 276){
						return;
					}
}}
// ==========================================
// ???????? (?? 1 ????,0 ????)
// ??:????????????????????
// ==========================================
uint8_t Check_Collision(int x1, int y1, int x2, int y2) {
    // ?? X ?? Y ?????
    int dx = x1 - x2;
    int dy = y1 - y2;
    
    // ???????,??????????? sqrt() ??!
    if ((dx * dx + dy * dy) < (34* 34)) {
        return 1; // ?? ????!
    }
    return 0;     // ??? ??
}
// ==========================================
// ???????
// ==========================================
void Game_Over() {
    char buf[64];
    game_started_easy = 0;
		game_started_mid = 0;
	  game_started_hard = 0;
    // 1. ??????????? Game Over!

    if (active_balls > max_score) {
        max_score = active_balls; // ???????
    }
		   SendToScreen("page over");
		sprintf(buf, "t0.txt=\"Max: %d\"", max_score);
    SendToScreen(buf);
    // 2. ?????(?????????????)
    active_balls = 0;
		memset(balls, 0, sizeof(balls));
 
    // SendToScreen("fill 0,0,320,240,0");
 
}

// ????????,??????
void Detect_All_Balls_Collision(void)
{
    if(active_balls < 2)  // ??2??,?????
    {
        is_collision = 0;
        return;
    }

    is_collision = 0;
    // ????:i ? j ????,??????
    for(int i = 0; i < active_balls; i++)
    {
        // ?????????(old_x/old_y ???????)
        int ball1_x = balls[i].old_x;
        int ball1_y = balls[i].old_y;

        for(int j = i + 1; j < active_balls; j++)
        {
            int ball2_x = balls[j].old_x;
            int ball2_y = balls[j].old_y;

            // ??????
            if(Check_Collision(ball1_x, ball1_y, ball2_x, ball2_y))
            {
                is_collision = 1;
                goto COLLISION_END; // ???????????
            }
        }
    }
COLLISION_END:
    return;
}

void Collision_Response(void)
{
    if(is_collision == 0)
        return;

    // 1. ?????(????Beep??)
    Beep_Play1(200);
    // 2. LED ??:???
    LED_Only_Red();
    // 3. ??:?????????
    Game_Over();
    // 4. ????
    is_collision = 0;
}



// ====================== ??? ======================
int main(void)
{
	max_score = 0;
    SYSCFG_DL_init();
    __enable_irq();
		LED_All_Off();
    // ?? UART0 ?????(??? SysConfig ???? RX Interrupt ????)
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
	

		
  
    /* ====== ????:?????? ====== */
    while (game_started_easy == 0&&game_started_mid == 0&&game_started_hard == 0) {
				
        // ?????????????,?????? 0x5A ??
				DL_GPIO_togglePins(GPIO_LEDS_PORT,GPIO_LEDS_USER_LED_1_PIN);
			delay_cycles(DELAY);
        
    }

    /* ====== ????:????,???????? ====== */
    // ?????????????
    SendToScreen("t0.txt=\"Score: 0\"");
		while(1){

							while(game_started_easy != 0){
										LED_Flow_Run();
										delay_cycles(DELAY);
											if(launch_ball == 1){
												// ????:?????????????
																if (active_balls < MAX_BALLS) {
																		
																		launch_forward(); // ????
																		
																		// ?? ????:??????????,?????????!
																		balls[active_balls] = (GameObject){120, 168, 107, 34, 34, 2, 0.05, 1.5708, -1, -1};
																	 
																		active_balls++;   // ??????? +1
																		char buf[64];
																		sprintf(buf, "t0.txt=\"Score: %d\"", active_balls);
																		SendToScreen(buf);
																}
																
																launch_ball = 0;
												}
										for(int i=0;i<active_balls;i++){
														 Rotate_Image(&balls[i]);
														}
										game_started_easy = 1;
										Detect_All_Balls_Collision();
										Collision_Response();
										
									}
							
									
									
								while(game_started_mid != 0){
										LED_Flow_Run();
										delay_cycles(DELAY);
											if(launch_ball == 1){
												// ????:?????????????
																int random_pic_id = (rand() % 3) + 2;
																if (active_balls < MAX_BALLS) {
																		
																		launch_forward(); // ????
																		
																		// ?? ????:??????????,?????????!
																		balls[active_balls] = Create_Ball(random_pic_id,0.05);
																	 
																		active_balls++;   // ??????? +1
																	char buf[64];
																		sprintf(buf, "t0.txt=\"Score: %d\"", active_balls);
																		SendToScreen(buf);
																}
																
																launch_ball = 0;
												}
										for(int i=0;i<active_balls;i++){
														 Rotate_Image(&balls[i]);
														}
										game_started_mid = 1;
										Detect_All_Balls_Collision();
										Collision_Response();
										 
									}
						while(game_started_hard != 0){
										LED_Flow_Run();
										delay_cycles(DELAY);
											if(launch_ball == 1){
												// ????:?????????????
																	int random_pic_id = (rand() % 3) + 2;
																if (active_balls < MAX_BALLS) {
																		
																		launch_forward(); // ????
																		
																		// ?? ????:??????????,?????????!
																		balls[active_balls] =  Create_Ball(random_pic_id,0.10);
																	 
																		active_balls++;   // ??????? +1
																	char buf[64];
																		sprintf(buf, "t0.txt=\"Score: %d\"", active_balls);
																		SendToScreen(buf);
																}
																
																launch_ball = 0;
												}
										for(int i=0;i<active_balls;i++){
														 Rotate_Image(&balls[i]);
														}
										game_started_hard = 1;
										Detect_All_Balls_Collision();
										Collision_Response();
										 
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
                game_started_easy = 1; // ?? 5A,????
            }
						 else if(rx_data == 0x5B) {
                game_started_mid = 1;  // ?? 4A,????
            }
						  else if(rx_data == 0x5C) {
               game_started_hard = 1;  // ?? 4A,????
            }
            else if(rx_data == 0x4A) {
                launch_ball = 1;  // ?? 4A,????
            }
            break;
        }
        default: break;
    }
	}