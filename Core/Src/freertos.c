/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint8_t uartBuffer[UARTBUFFER];
float PWMValue[2];
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
    /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
osMutexDef(printfMutex);
osMutexId printfMutex;
void UARTTask(void const *argument) {
    int i;
    int array_empty_flag = 1;
    
    printfMutex = osMutexCreate(osMutex(printfMutex));

    uart_printf("Uart Task Start.\r\n");
    for (;;) {
        HAL_UART_Receive_DMA(&huart1, &uartBuffer[0], UARTBUFFER);
        for (i = 0; i < UARTBUFFER; i++) {
            if (uartBuffer[i] != 0) {
                array_empty_flag = 0;
                break;
            }
        }
        if (array_empty_flag == 0) {
            array_empty_flag = 1;
            /*
             * example:
             * UartInput: x:+003,
             * PWMValue[1] = 3;
             * UartInput: x:+012,
             * PWMValue[1] = 12;
             * */
            ReformatBuffer(uartBuffer, PWMValue);
            uart_printf("PWM Value: %0.2f.\r\n", PWMValue[0]);

            memset(uartBuffer, 0, UARTBUFFER);

            HAL_UART_DMAStop(&huart1);
        }
        osDelay(100);
    }

}

void PWMTask(void const *argument) {
//    uart_printf("PWM Task Start.\r\n");
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    for (;;) {
                __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, PWMValue[0] * 100);
        osDelay(10);
    }
}

void KEYTask(void const *argument) {
//    uart_printf("KEY Task Start.\r\n");

    button_init(&KEY1, read_KEY1_GPIO, 0);
    button_init(&KEY2, read_KEY2_GPIO, 0);
    button_attach(&KEY1, PRESS_DOWN, KEY1_PRESS_DOWN_Handler);
    button_attach(&KEY2, PRESS_DOWN, KEY2_PRESS_DOWN_Handler);
    button_start(&KEY1);
    button_start(&KEY2);

    for (;;) {
        button_ticks();
        osDelay(5);
    }
}
/* USER CODE END Application */
