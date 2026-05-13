#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifndef LCD_RS_GPIO_Port
#define LCD_RS_GPIO_Port GPIOB
#endif
#ifndef LCD_RS_Pin
#define LCD_RS_Pin GPIO_PIN_0
#endif

#ifndef LCD_E_GPIO_Port
#define LCD_E_GPIO_Port GPIOB
#endif
#ifndef LCD_E_Pin
#define LCD_E_Pin GPIO_PIN_1
#endif

#ifndef LCD_D4_GPIO_Port
#define LCD_D4_GPIO_Port GPIOB
#endif
#ifndef LCD_D4_Pin
#define LCD_D4_Pin GPIO_PIN_2
#endif

#ifndef LCD_D5_GPIO_Port
#define LCD_D5_GPIO_Port GPIOB
#endif
#ifndef LCD_D5_Pin
#define LCD_D5_Pin GPIO_PIN_10
#endif

#ifndef LCD_D6_GPIO_Port
#define LCD_D6_GPIO_Port GPIOB
#endif
#ifndef LCD_D6_Pin
#define LCD_D6_Pin GPIO_PIN_13
#endif

#ifndef LCD_D7_GPIO_Port
#define LCD_D7_GPIO_Port GPIOB
#endif
#ifndef LCD_D7_Pin
#define LCD_D7_Pin GPIO_PIN_12
#endif

static inline void lcd_dwt_init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline void lcd_delay_us(uint32_t us)
{
  const uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;
  const uint32_t start = DWT->CYCCNT;
  const uint32_t ticks = us * cycles_per_us;
  while ((DWT->CYCCNT - start) < ticks) { ; }
}

static inline void lcd_pulse_e(void)
{
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_SET);
  lcd_delay_us(2);
  HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, GPIO_PIN_RESET);
  lcd_delay_us(50);
}

static inline void lcd_write4(uint8_t nibble)
{
  HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  lcd_pulse_e();
}

static inline void lcd_send(uint8_t value, uint8_t rs)
{
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
  lcd_write4((uint8_t)((value >> 4) & 0x0F));
  lcd_write4((uint8_t)(value & 0x0F));
}

static inline void lcd_cmd(uint8_t cmd)
{
  lcd_send(cmd, 0);
  if (cmd == 0x01 || cmd == 0x02) HAL_Delay(2);
}

static inline void lcd_data(uint8_t data)
{
  lcd_send(data, 1);
}

static inline void lcd_init(void)
{
  lcd_dwt_init();
  HAL_Delay(20);

  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_E_GPIO_Port,  LCD_E_Pin,  GPIO_PIN_RESET);

  lcd_write4(0x03); HAL_Delay(5);
  lcd_write4(0x03); lcd_delay_us(150);
  lcd_write4(0x03); lcd_delay_us(150);

  lcd_write4(0x02); lcd_delay_us(150);

  lcd_cmd(0x28);
  lcd_cmd(0x08);
  lcd_cmd(0x01);
  lcd_cmd(0x06);
  lcd_cmd(0x0C);
}

static inline void lcd_clear(void)
{
  lcd_cmd(0x01);
}

static inline void lcd_set_cursor(uint8_t row, uint8_t col)
{
  const uint8_t addr = (row == 0) ? (uint8_t)(0x00 + col) : (uint8_t)(0x40 + col);
  lcd_cmd((uint8_t)(0x80 | addr));
}

static inline void lcd_print(const char *s)
{
  while (*s) lcd_data((uint8_t)*s++);
}

static inline void lcd_print_int(int v)
{
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", v);
  lcd_print(buf);
}

#endif /* INC_LCD_H_ */
