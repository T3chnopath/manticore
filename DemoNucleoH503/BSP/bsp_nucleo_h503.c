#include "bsp_nucleo_h503.h"
#include "utility.h"
#include "tx_api.h"

static void _BSP_SystemClockConfig(void);
static void _BSP_ErrorHandler(void);
static void _BSP_GPIO_Init(void);
static void _BSP_FDCAN_Init(void);
static void _BSP_UART_Init(void);

static const uint16_t BSP_CLK_DELAY_MS = 500;
static const uint16_t BSP_DELAY_MS = 1000;

UART_HandleTypeDef ConsoleUart;

void BSP_Init(void)
{
    HAL_Init();
    _BSP_SystemClockConfig();
    tx_thread_sleep(BSP_CLK_DELAY_MS);
    
    _BSP_GPIO_Init();
    _BSP_FDCAN_Init();
    _BSP_UART_Init();
    tx_thread_sleep(BSP_DELAY_MS);
}

static void _BSP_SystemClockConfig(void)
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
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State     = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLL1_SOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 12;
    RCC_OscInitStruct.PLL.PLLN = 250;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1_VCIRANGE_1;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
    RCC_OscInitStruct.PLL.PLLFRACN  = 0;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        _BSP_ErrorHandler();
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
      _BSP_ErrorHandler();
    }
}

static void _BSP_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // GPIO Ports Clock Enable
    GPIO_PortClkEnable(LED_GREEN_GPIO_Port);

    // Configure GPIO pin Output Level
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

    // Configure GPIO pin : LED1_GREEN_Pin
    GPIO_InitStruct.Pin = LED_GREEN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);
}

static void _BSP_FDCAN_Init(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Config FDCAN clock to be 32MHz
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.PLL2.PLL2Source = RCC_PLL2_SOURCE_HSE;
    PeriphClkInitStruct.PLL2.PLL2M = 2;
    PeriphClkInitStruct.PLL2.PLL2N = 32;
    PeriphClkInitStruct.PLL2.PLL2P = 2;
    PeriphClkInitStruct.PLL2.PLL2Q = 4;
    PeriphClkInitStruct.PLL2.PLL2R = 2;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2_VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2_VCORANGE_WIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    PeriphClkInitStruct.PLL2.PLL2ClockOut = RCC_PLL2_DIVQ;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL2Q;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      _BSP_ErrorHandler();
    }

    // Peripheral Clock Enable 
    __HAL_RCC_FDCAN_CLK_ENABLE();
    GPIO_PortClkEnable(FDCAN_TX_Port);
    GPIO_PortClkEnable(FDCAN_RX_Port);

    // FDCAN GPIO Configuration
    GPIO_InitStruct.Pin = FDCAN_TX_Pin | FDCAN_RX_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // Select alternate function based on FDCAN interface
#if defined(FDCAN1_EN)
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
#endif

    HAL_GPIO_Init(FDCAN_TX_Port, &GPIO_InitStruct);
    HAL_GPIO_Init(FDCAN_RX_Port, &GPIO_InitStruct);

    // Interrupt init, default to IT0, preempt = 2, subpriority = 0 
#if defined(FDCAN1_EN)
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
#endif 
}

static void _BSP_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};    

    // UART3 CLK Enable 
#ifdef UART3_EN
    __HAL_RCC_USART3_CLK_ENABLE();
#endif 

#ifdef UART3_EN
    ConsoleUart.Instance            = USART3;
#endif
    ConsoleUart.Init.BaudRate       = UART_BAUDRATE;
    ConsoleUart.Init.WordLength     = UART_WORDLENGTH_8B;
    ConsoleUart.Init.StopBits       = UART_STOPBITS_1;
    ConsoleUart.Init.Parity         = UART_PARITY_NONE;
    ConsoleUart.Init.Mode           = UART_MODE_TX_RX;
    ConsoleUart.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
    ConsoleUart.Init.OverSampling   = UART_OVERSAMPLING_16;
    ConsoleUart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    ConsoleUart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    ConsoleUart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&ConsoleUart) != HAL_OK)
    {
        _BSP_ErrorHandler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&ConsoleUart, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        _BSP_ErrorHandler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&ConsoleUart, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        _BSP_ErrorHandler();
    }
    if (HAL_UARTEx_DisableFifoMode(&ConsoleUart) != HAL_OK)
    {
        _BSP_ErrorHandler();
    }

#ifdef UART3_EN
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
#endif
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        _BSP_ErrorHandler();
    }

    //Enable GPIOA Clock
    GPIO_PortClkEnable(UART_RX_Port);
    GPIO_PortClkEnable(UART_TX_Port);

    // UART3 GPIO Configuration
    GPIO_InitStruct.Pin   = UART_TX_Pin|UART_RX_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
#ifdef UART3_EN
    GPIO_InitStruct.Alternate = GPIO_AF13_USART3;
#endif
    HAL_GPIO_Init(UART_TX_Port, &GPIO_InitStruct);
    HAL_GPIO_Init(UART_RX_Port, &GPIO_InitStruct);

    // Rx interrupt
#ifdef UART3_EN
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
#endif
}

static void _BSP_ErrorHandler(void)
{
    __disable_irq();
    while (1)
    {
        // NOP
    }
}