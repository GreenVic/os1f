#include "stm32746g_discovery.h"

static void
CPU_CACHE_Enable(void);
static void
SystemClock_Config(void);

void HAL_MspInit(void)
{
	CPU_CACHE_Enable();
	SystemClock_Config();
	BSP_LED_Init(LED1);
	BSP_LED_On(LED1);
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 216000000
 *            HCLK(Hz)                       = 216000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 25
 *            PLL_N                          = 432
 *            PLL_P                          = 2
 *            PLL_Q                          = 9
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 7
 * @param  None
 * @retval None
 */

static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
//	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
//	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
//	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
//	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
//	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
//	RCC_OscInitStruct.PLL.PLLM = 25;
//	RCC_OscInitStruct.PLL.PLLN = 432;
//	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
//	RCC_OscInitStruct.PLL.PLLQ = 9;
//	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (ret != HAL_OK)
		goto FAIL;

	/* copied in from stm32f7xx_hal_rcc.c lines 374-423 & 601 - 666 */
	/* Set the new HSE configuration ---------------------------------------*/
	__HAL_RCC_HSE_CONFIG(RCC_HSE_ON);

	/* Get Start Tick*/
	uint32_t tickstart = HAL_GetTick();

	/* Wait till HSE is ready */
	while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET)
		if ((HAL_GetTick() - tickstart) > HSE_TIMEOUT_VALUE)
			goto FAIL;

	/*-------------------------------- PLL Configuration -----------------------*/
	/* Check if the PLL is used as system clock or not */
	if (__HAL_RCC_GET_SYSCLK_SOURCE() != RCC_SYSCLKSOURCE_STATUS_PLLCLK)
	{
		/* Disable the main PLL. */
		__HAL_RCC_PLL_DISABLE();

		/* Get Start Tick*/
		tickstart = HAL_GetTick();

		/* Wait till PLL is ready */
		while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) != RESET)
			if ((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
				goto FAIL;

		/* Configure the main PLL clock source, multiplication and division factors. */
		__HAL_RCC_PLL_CONFIG(RCC_PLLSOURCE_HSE, 25, 432, RCC_PLLP_DIV2, 9);

		/* Enable the main PLL. */
		__HAL_RCC_PLL_ENABLE();

		/* Get Start Tick*/
		tickstart = HAL_GetTick();

		/* Wait till PLL is ready */
		while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET)
			if ((HAL_GetTick() - tickstart) > PLL_TIMEOUT_VALUE)
				goto FAIL;

	}

	/* Activate the OverDrive to reach the 216 MHz Frequency */
	ret = HAL_PWREx_EnableOverDrive();
	if (ret != HAL_OK)
		goto FAIL;

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
	if (ret != HAL_OK)
		goto FAIL;

	return;

	FAIL: while (1)
	{
	};
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}
