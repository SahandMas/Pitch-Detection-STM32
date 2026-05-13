/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026
  * All rights reserved.
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include "lcd.h"

#include <stdio.h>
#include <string.h>

/* USER CODE BEGIN PTD */
typedef struct {
  const char *name;
  uint32_t f_cHz;
} note_t;
/* USER CODE END PTD */

/* USER CODE BEGIN PD */
#define FS_HZ                 8000
#define BUF_LEN               512

#define LAG_MIN               23
#define LAG_MAX               94

#define PEAK_ALPHA_NUM        3
#define PEAK_ALPHA_DEN        10

#define SILENCE_MAXABS_TH     25
#define SILENCE_ENERGY_TH     200000
/* USER CODE END PD */

/* Private variables*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;   // 

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
static uint16_t adc_buf[BUF_LEN];
static uint16_t adc_idx = 0;
static volatile uint8_t  frame_ready = 0;

static int16_t  xbuf[BUF_LEN];
static int64_t  rbuf[LAG_MAX + 1];

static uint16_t adc_sample = 0;

static const note_t note_table[] = {
  {"E2",  8241}, {"F2",  8731}, {"F#2",  9250}, {"G2",  9800}, {"G#2", 10383},
  {"A2", 11000}, {"A#2",11654}, {"B2", 12347}, {"C3", 13081}, {"C#3",13859},
  {"D3", 14683}, {"D#3",15556}, {"E3", 16481}, {"F3", 17461}, {"F#3",18500},
  {"G3", 19600}, {"G#3",20765}, {"A3", 22000}, {"A#3",23308}, {"B3", 24694},
  {"C4", 26163}, {"C#4",27718}, {"D4", 29366}, {"D#4",31113}, {"E4", 32963},
  {"F4", 34923}
};
static const int NOTE_TABLE_LEN = (int)(sizeof(note_table) / sizeof(note_table[0]));
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);          // 
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
static void uart_print(const char *s);
static uint32_t estimate_f0_cHz(const uint16_t *adc, uint32_t *out_lag);
static const char* map_note(uint32_t f0_cHz);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
static void uart_print(const char *s)
{
  HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

static uint32_t estimate_f0_cHz(const uint16_t *adc, uint32_t *out_lag)
{
  int32_t sum = 0;
	int32_t V_sum =0;
  for (int i = 0; i < BUF_LEN; i++) sum += (int32_t)adc[i];
      const int32_t mean = sum / BUF_LEN;
  int32_t maxabs = 0;
  int64_t energy = 0;
  int32_t curtrh = 0;
  for (int i = 0; i < BUF_LEN; i++) {
    int32_t v = (int32_t)adc[i] - mean;
    //int32_t v = (int32_t)adc[i];
    
    if (v >  32767) v =  32767;
    if (v < -32768) v = -32768;
    V_sum += (v>=0) ? v : -v;
    xbuf[i] = (int16_t)v;
    
    const int32_t av = (v >= 0) ? v : -v;
    if (av > maxabs) maxabs = av;

    energy += v * v;
  }
	

  if (maxabs < SILENCE_MAXABS_TH || energy < SILENCE_ENERGY_TH) {
    *out_lag = 0;
    return 0;
  }
	
  for (int lag = 1; lag <= LAG_MAX; lag++) rbuf[lag] = 0;

  for (int lag = LAG_MIN; lag <= LAG_MAX; lag++) {
    int64_t acc = 0;
    for (int n = lag; n < BUF_LEN; n++) {
      acc += (int64_t)xbuf[n] * (int64_t)xbuf[n - lag];
    }
    rbuf[lag] = acc;
  }
	uint16_t idx_GLOBMax =0 ;
	uint64_t GLOBMax =0 ;
	for ( uint16_t i =LAG_MIN ; i< LAG_MAX;i++){
		uint32_t temp = (rbuf[i] > 0) ? rbuf[i] : -rbuf[i];
		if(temp > GLOBMax){
			GLOBMax = temp;
			idx_GLOBMax = i;
		}
	}
	*out_lag = idx_GLOBMax;
  return (uint32_t)((FS_HZ * 100U + (idx_GLOBMax / 2U)) / (uint32_t)idx_GLOBMax);
  
  //curtrh = rbuf[LAG_MIN];
  // int64_t thr = (rbuf[0] * PEAK_ALPHA_NUM) / PEAK_ALPHA_DEN;
  //int32_t thr = GLOBMax - 1;
  //int best_lag = 0;
  //for (int lag = LAG_MIN + 1; lag <= LAG_MAX - 1; lag++) {
  //for (int lag = LAG_MAX - 1; lag >= LAG_MIN + 1; lag--) {  
	  //const int64_t a = rbuf[lag - 1];
    //const int64_t b = rbuf[lag];
    //const int64_t c = rbuf[lag + 1];
    //curtrh += b;
		//curtrh /=2;
  //  if (b > thr && b > 0 && b > a && b >= c) {
    //if (b > 0 &&  ) {    
		//best_lag = lag;
      //break;
    //}
  //}
	
	//lcd_clear();
	//char line1[17];
	//sprintf(line1,"%lld",energy);
	//lcd_set_cursor(0, 0);
  //lcd_print(line1);
  
  //if (best_lag == 0) {
    //*out_lag = 0;
    //return 0;
 // }
	
	//*out_lag = (uint32_t)best_lag;
  //return (uint32_t)((FS_HZ * 100U + (best_lag / 2U)) / (uint32_t)best_lag);
}

static const char* map_note(uint32_t f0_cHz)
{
  if (f0_cHz == 0) return "---";

  int best_idx = 0;
  uint32_t best_diff = 0xFFFFFFFF;

  for (int i = 0; i < NOTE_TABLE_LEN; i++) {
    const uint32_t f = note_table[i].f_cHz;
    const uint32_t diff = (f > f0_cHz) ? (f - f0_cHz) : (f0_cHz - f);

    if (diff < best_diff) {
      best_diff = diff;
      best_idx = i;
    }
  }
  return note_table[best_idx].name;
}

/* ADC interrupt-driven buffering */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance != ADC1) return;
  if (frame_ready) return;

  adc_buf[adc_idx++] = HAL_ADC_GetValue(&hadc1);

  if (adc_idx >= BUF_LEN) {
    frame_ready = 1;
    adc_idx = 0;

    //HAL_ADC_Stop_IT(&hadc1);
    __HAL_ADC_DISABLE_IT(&hadc1, ADC_IT_EOC);

    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
    return;
  }

  /*(if (HAL_ADC_Start_IT(&hadc1) != HAL_OK) {
    
  }*/
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  (void)hadc;
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();           // 
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  uart_print("System started.\r\n");

  lcd_init();
  lcd_clear();
  //lcd_set_cursor(0, 0);
  //lcd_print("Pitch Detector");
  //lcd_set_cursor(1, 0);
  //lcd_print("Fs=8k N=320");
  //HAL_Delay(800);
  //lcd_clear();

  if (HAL_TIM_Base_Start(&htim2) != HAL_OK) Error_Handler();

  adc_idx = 0;
  frame_ready = 0;
  if (HAL_ADC_Start_IT(&hadc1) != HAL_OK) Error_Handler();

  while (1)
  {
    if (!frame_ready) continue;
	  
		
		lcd_clear();
    uint32_t lag = 0;
    const uint32_t f0_cHz = estimate_f0_cHz((const uint16_t*)adc_buf, &lag);
    const char *note = map_note(f0_cHz);

    char line1[17];
    char line2[17];

    if (f0_cHz == 0) {
      snprintf(line1, sizeof(line1), "No Signal      ");
      snprintf(line2, sizeof(line2), "Note: ---      ");
    } else {
      const uint32_t hz_int = f0_cHz / 100U;
      const uint32_t hz_dec = f0_cHz % 100U;

      snprintf(line1, sizeof(line1), "F0=%3lu.%02luHz   ",
               (unsigned long)hz_int, (unsigned long)hz_dec);

      snprintf(line2, sizeof(line2), "Note:%-4s lag=%2lu",
               note, (unsigned long)lag);
    }
		//sprintf(line1,"%d",f0_cHz);
		
    lcd_set_cursor(0, 0);
    lcd_print(line1);
    lcd_set_cursor(1, 0);
    lcd_print(line2);

    char msg[80];
    if (f0_cHz == 0) {
      snprintf(msg, sizeof(msg), "unvoiced\r\n");
    } else {
      snprintf(msg, sizeof(msg), "F0=%lu.%02luHz note=%s lag=%lu\r\n",
               (unsigned long)(f0_cHz / 100U),
               (unsigned long)(f0_cHz % 100U),
               note,
               (unsigned long)lag);
    }
    uart_print(msg);

    frame_ready = 0;
    adc_idx = 0;

    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC | ADC_FLAG_OVR);
    __HAL_ADC_ENABLE_IT(&hadc1, ADC_IT_EOC);

    if (HAL_ADC_Start_IT(&hadc1) != HAL_OK) {
      Error_Handler();
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;

  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;

  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();

}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK) Error_Handler();
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;

  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 10499;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();
}

static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOB,
                    GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13,
                    GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  // Optional debug LED pin PC8 (if you have it wired/configured)
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { ; }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file; (void)line;
}
#endif
