#include "bsp_nucleo_h563.h"

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while( !__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY) ) 
    {
        // NOP
    }

    // Initializes the RCC Oscillators according to the specified parameters in the RCC_OscInitTypeDef structure.
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48
                                     | RCC_OSCILLATORTYPE_HSI
                                     | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIGITAL;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv   = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI48State    = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 250;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1_VCIRANGE_1;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
    RCC_OscInitStruct.PLL.PLLFRACN  = 0;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    // Initializes the CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_PCLK3;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
      Error_Handler();
    }
}


void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // GPIO Ports Clock Enable
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();

    // Configure GPIO pin Output Level
    HAL_GPIO_WritePin(LED2_YELLOW_GPIO_Port, LED2_YELLOW_Pin, GPIO_PIN_RESET);

    // Configure GPIO pin Output Level
    HAL_GPIO_WritePin(LED1_GREEN_GPIO_Port, LED1_GREEN_Pin, GPIO_PIN_RESET);

    // Configure GPIO pin Output Level
    HAL_GPIO_WritePin(LED3_RED_GPIO_Port, LED3_RED_Pin, GPIO_PIN_RESET);

    // Configure GPIO pin : USER_BUTTON_Pin
    GPIO_InitStruct.Pin = USER_BUTTON_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

    // Configure GPIO pin : LED2_YELLOW_Pin
    GPIO_InitStruct.Pin = LED2_YELLOW_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED2_YELLOW_GPIO_Port, &GPIO_InitStruct);

    // Configure GPIO pin : LED1_GREEN_Pin
    GPIO_InitStruct.Pin = LED1_GREEN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED1_GREEN_GPIO_Port, &GPIO_InitStruct);

    // Configure GPIO pin : LED3_RED_Pin
    GPIO_InitStruct.Pin = LED3_RED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED3_RED_GPIO_Port, &GPIO_InitStruct);

    // Configure GPIO pin : UCPD_FLT_Pin
    GPIO_InitStruct.Pin = UCPD_FLT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(UCPD_FLT_GPIO_Port, &GPIO_InitStruct);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
        // NOP
    }
}