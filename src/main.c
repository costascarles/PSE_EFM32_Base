#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "my_i2c.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "croutine.h"

#include "em_chip.h"
#include "bsp.h"
#include "bsp_trace.h"

#include "sleep.h"

#define STACK_SIZE_FOR_TASK    (configMINIMAL_STACK_SIZE + 10)
#define TASK_PRIORITY          (tskIDLE_PRIORITY + 1)
#define COLA_TAMANO 5

QueueHandle_t colaGiroX;
QueueHandle_t colaGiroY;

QueueHandle_t colaAccX;
QueueHandle_t colaAccY;

QueueHandle_t direction;
/* Structure with parameters for LedBlink */


static void readGiroX(){
  int8_t valx_l, valx_h;
  int16_t x;
  while(1){
    // Init Giro
    my_i2c_write(0x10,0x20);
    // Read X
    my_i2c_read(0x18, &valx_l);
    my_i2c_read(0x19, &valx_h);
    x = (valx_h << 8) | valx_l;
    xQueueSend(colaGiroX, &x, portMAX_DELAY);
  }
}
static void readGiroY(){
  int8_t valy_l, valy_h;
  int16_t y;
  while(1){
    // Init Giro
    my_i2c_write(0x10,0x20);
    // Read X
    my_i2c_read(0x1A, &valy_l);
    my_i2c_read(0x1B, &valy_h);
    y = (valy_h << 8) | valy_l;
    xQueueSend(colaGiroY, &y, portMAX_DELAY);
  }
}

static void readAX(){
  int8_t valx_l, valx_h;
  int16_t x;
  while(1){
	 my_i2c_write(0x20,1 << 6);
	 my_i2c_read(0x28,&valx_l);
	 my_i2c_read(0x29,&valx_h);
	 x = (valx_h << 8) | valx_l;
	 xQueueSend(colaAccX, &x, portMAX_DELAY);
  }
}
static void readAY(){
  int8_t valy_l, valy_h;
  int16_t y;
  while(1){
	 my_i2c_write(0x20,1 << 6);
	 my_i2c_read(0x2A,&valy_l);
	 my_i2c_read(0x2B,&valy_h);
	 y = (valy_h << 8) | valy_l;
	 xQueueSend(colaAccY, &y, portMAX_DELAY);
  }


}
static void calcDirection(){
	while(1){

		int16_t y;
		int8_t dir;
		xQueueReceive(colaGiroY, &y, portMAX_DELAY);
		if(y<0){
			dir = 0;
			//printf("Left\n");
		}else{
			dir=1;
			//printf("Right\n");
		}
		xQueueSend(direction, &dir, portMAX_DELAY);
		//printf("VALOR valor X: %d\n", x);
		//printf("VALOR valor Y: %d\n", y);
	}
}

static void printTOT(){
	int game = 0;
	int num=0;
while(1){
	int8_t dir;
	 xQueueReceive(direction, &dir, portMAX_DELAY);

	 if(game == 0){
		  num = (rand() % (1 - 0 + 1)) + 0;
		 if(num==1){
			 printf("Dreta\n");
		 }else{
			 printf("Esquerra\n");
		 }
		 game=1;
	 }
	 if(game == 1){
		 if(dir==num){
			 printf("Molt Bee!!!\n");
			 game=0;
		 }

	 }

	 }
}

/***************************************************************************//**
 * @brief  Main function
 ******************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();
  /* If first word of user data page is non-zero, enable Energy Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize LED driver */
  BSP_LedsInit();
  /* Setting state of leds*/
  BSP_LedSet(0);
  BSP_LedSet(1);

  /* Initialize SLEEP driver, no calbacks are used */
  SLEEP_Init(NULL, NULL);
#if (configSLEEP_MODE < 3)
  /* do not let to sleep deeper than define */
  SLEEP_SleepBlockBegin((SLEEP_EnergyMode_t)(configSLEEP_MODE + 1));
#endif

  /* Parameters value for tasks*/
 // static TaskParams_t parametersToTask1 = { pdMS_TO_TICKS(1000), 0 };


  /* Create two tasks for blinking leds*/
 // xTaskCreate(LedBlink, (const char *) "LedBlink1", STACK_SIZE_FOR_TASK, &parametersToTask1, TASK_PRIORITY, NULL);

  my_i2c_init(0xD7);
  uint8_t WIAM;
  my_i2c_read(0x0F, &WIAM);
  if(WIAM == 0x68){
	direction = xQueueCreate(COLA_TAMANO, sizeof(uint8_t));
    colaGiroY = xQueueCreate(COLA_TAMANO, sizeof(uint16_t));
    //colaAccX = xQueueCreate(COLA_TAMANO, sizeof(uint16_t));
   // colaAccY = xQueueCreate(COLA_TAMANO, sizeof(uint16_t));
    xTaskCreate(readGiroX, (const char *) "readGiroX2", STACK_SIZE_FOR_TASK ,NULL , TASK_PRIORITY, NULL);
    xTaskCreate(readGiroY, (const char *) "readGiroY2", STACK_SIZE_FOR_TASK ,NULL,  TASK_PRIORITY, NULL);
    xTaskCreate(calcDirection, (const char *) "calcDirectionX2", STACK_SIZE_FOR_TASK , NULL , TASK_PRIORITY, NULL);
    //xTaskCreate(readAX, (const char *) "readAX2", STACK_SIZE_FOR_TASK,NULL ,  TASK_PRIORITY, NULL);
   // xTaskCreate(readAY, (const char *) "readAY2", STACK_SIZE_FOR_TASK, NULL , TASK_PRIORITY, NULL);

    xTaskCreate(printTOT, (const char *) "printTOTX2", STACK_SIZE_FOR_TASK, NULL , TASK_PRIORITY, NULL);
    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
   }





   return 0;
 }


