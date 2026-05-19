/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "stm32f1xx_hal.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "key.h"
#include "oled.h"
#include "ws2812b.h"
#include "encoder.h"
#include "custom_input_mapper.h"
#include "controller_to_keymouse.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void MyKeyCallback(KeyEvent_t event, uint32_t param);      // 按键事件回调函数：根据事件类型发送串口数据、控制RGB等
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint8_t key_param = 0;           // 按键事件参数：0-9分别对应9个按键，0表示无效
static uint8_t encoder_param = 0;       // 编码器事件参数：0-2分别对应3个编码器状态，0表示无效，1表示进入买弹面板,2表示进入确认界面 
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  encoder_init();
  OLED_Init();
  ws2812b_init();
  Key_Init();
  custom_input_init();
  Key_RegisterCallback(MyKeyCallback);
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */
  OLED_ColorTurn(0);              // 正常显示
  OLED_DisplayTurn(0);            // 不翻转
  OLED_ShowNum(0, 0, 42, 2, 16, 0);
  OLED_ShowString(16, 0, "mmAmmo:", 16, 0);
  OLED_Refresh();  // 刷新OLED显示
  int32_t last_total = 0;
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    switch (key_param) {
      case 0:   break;
      case 1:   // 发送1弹操作：按下1弹键，点击对应位置，发送确认序列，点亮LED
        buy_42mmammo(1);
        
        key_param = 0; 
        break;  
      case 2:   // 发送5弹操作：按下5弹键，点击对应位置，发送确认序列，点亮LED 
        buy_42mmammo(5);
        
        key_param = 0; 
        break;
      case 3:   // 发送10弹操作：按下10弹键，点击对应位置，发送确认序列，点亮LED
        buy_42mmammo(10);
        
        key_param = 0; 
        break;
      case 4:   
        
        
        key_param = 0; 
        break;
      case 5:  
      

        key_param = 0; 
        break;
      case 6:  
        
        
        key_param = 0; 
        break;
      case 7:  
        
        
        key_param = 0; 
        break;
      case 8:  
        
        
        key_param = 0; 
        break;
      case 9:
        
        key_param = 0; 
        break;
      default:  key_param = 0; break;
    
    }

    if (encoder_param == 0) {
      // 无效状态，等待事件触发
    } else if (encoder_param == 1) {
      buy_42mmammo(last_total);  // 进入买弹面板：点击对应位置，发送打开购买界面序列
      encoder_param = 0;   // 重置编码器事件参数，等待旋转事件

      for (int i = 0; i < NUM_LEDS; i++) {
          ws2812b_set_color(i, 0, 0, 255); 
        }
        ws2812b_show();  // 刷新LED显示

    } else if (encoder_param == 2) {     

      //等待旋转编码器事件：根据旋转步数计算购买数量，发送购买操作，更新OLED显示      
      int16_t step = encoder_get_count();
      int32_t total = encoder_get_total_count();
      if (step != 0) {         
          last_total += step;  // 每步增加1弹
          if (last_total < 0) last_total = 0;  // 不允许负数
          if (last_total > 80) last_total = 80;  // 最大80弹       
      }

      OLED_ShowNum(72, 0, last_total, 3, 16, 0);  // 在OLED上显示弹药数量
      OLED_Refresh();  // 刷新OLED显示
      
    } else {
      encoder_param = 0;   // 重置编码器事件参数
    } 
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

//定时器中断回调函数：每 5ms 调用一次按键扫描函数
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        Key_UpdateTick();   // 更新时间戳
        Key_TimerScan();   // 每 5ms 调用一次按键扫描
    }
}

//外部中断回调函数：将外部中断事件传递给按键处理函数
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//     if(GPIO_Pin == GPIO_PIN_11) {   // 处理按键事件
//         if (encoder_param == 0) {
//           send_key_down(VK_O);  // 发送按键操作（如打开购买界面） 
//           encoder_param = 1;   // 进入买弹面板
//         }else if (encoder_param == 1) {
//           encoder_param = 2;   // 进入确认界面
//         }else {
//           encoder_param = 0;   // 重置编码器事件参数
//         }
//     }
// }

// 按键事件回调函数：根据事件类型发送串口数据、控制RGB等
void MyKeyCallback(KeyEvent_t event, uint32_t param) {
  switch (event) {
        case KEY_EVENT_PRESS:        // 短按
            switch (param) {
              case 0:  key_param = 1; break;
              case 1:  key_param = 2; break;
              case 2:  key_param = 3; break;
              case 3:  key_param = 4; break;
              case 4:  key_param = 5; break;
              case 5:  key_param = 6; break;
              case 6:  key_param = 7; break;
              case 7:  key_param = 8; break;
              case 8:  key_param = 9; break;
              case 9:  encoder_param = 1; break;
              default:  key_param = 0; break;
            }            
            break;
        case KEY_EVENT_LONG_PRESS:    // 长按
            
            break;
        case KEY_EVENT_REPEAT:        // 长按重复触发
            
            break;
        case KEY_EVENT_DOUBLE_CLICK:  // 双击事件
            if (param == 9) {
              encoder_reset_count();  // 重置编码器计数，准备检测旋转事件
              encoder_param = 2;  // 编码器按键双击事件
              for (int i = 0; i < NUM_LEDS; i++) {
                ws2812b_set_color(i, 255, 0, 255); 
              }
              ws2812b_show();  // 刷新LED显示
            } else {
              key_param = 0;   // 其他按键不处理双击事件
            }
            break;
        
        case KEY_EVENT_RELEASE:
            // 释放可做额外处理
            break;
        case KEY_EVENT_COMBO:         // 组合键，param为bitmask
            
            break;
        default:
            break;
    }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
