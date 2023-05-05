#include <stdio.h>
#include <stdlib.h>

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

/* Structure with parameters for LedBlink */
typedef struct {
  /* Delay between blink of led */
  TickType_t delay;
  /* Number of led */
  int          ledNo;
} TaskParams_t;

/***************************************************************************//**
 * @brief Simple task which is blinking led
 * @param *pParameters pointer to parameters passed to the function
 ******************************************************************************/
static void LedBlink(void *pParameters)
{
  TaskParams_t     * pData = (TaskParams_t*) pParameters;
  const TickType_t delay = pData->delay;

  for (;; ) {
    BSP_LedToggle(pData->ledNo);
    vTaskDelay(delay);
  }
}

static void readGiroX(void *pParameters){
  uint8_t valx_l, valx_h;
  uint16_t x;
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
static void readGiroY(void *pParameters){
  uint8_t valy_l, valy_h;
  uint16_t y;
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
static void printTOT(void *pParameters){
while(1){
	  uint16_t x;
	  uint16_t y;
	 xQueueReceive(colaGiroX, &x, portMAX_DELAY);
	 xQueueReceive(colaGiroY, &y, portMAX_DELAY);
	 printf("VALOR valor X: %d\n", x);
	 printf("VALOR valor Y: %d\n", y);
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
  static TaskParams_t parametersToTask2 = { pdMS_TO_TICKS(500), 1 };

  /* Create two tasks for blinking leds*/
 // xTaskCreate(LedBlink, (const char *) "LedBlink1", STACK_SIZE_FOR_TASK, &parametersToTask1, TASK_PRIORITY, NULL);

  my_i2c_init(0xD7);
  uint8_t WIAM;
  my_i2c_read(0x0F, &WIAM);
  if(WIAM == 0x68){
    colaGiroX = xQueueCreate(COLA_TAMANO, sizeof(uint16_t));
    colaGiroY = xQueueCreate(COLA_TAMANO, sizeof(uint16_t));
    xTaskCreate(readGiroX, (const char *) "readGiroX2", STACK_SIZE_FOR_TASK, &parametersToTask2, TASK_PRIORITY, NULL);
    xTaskCreate(readGiroY, (const char *) "readGiroY2", STACK_SIZE_FOR_TASK, &parametersToTask2, TASK_PRIORITY, NULL);
    xTaskCreate(printTOT, (const char *) "printTOTX2", STACK_SIZE_FOR_TASK, &parametersToTask2, TASK_PRIORITY, NULL);
    /*Start FreeRTOS Scheduler*/
    vTaskStartScheduler();
   }





   return 0;
 }


