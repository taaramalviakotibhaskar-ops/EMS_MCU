// The first 2 activities are hardware setup so aren't code related by activity 3 is

// Activity 3: LCD Display of SID (Student Identification)

// Part 1: LCD Header :)
#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "main.h"  // For hi2c1 handle

#define LCD_I2C_ADDR 0x27 << 1  // 0x4E for write (shifted)

void lcd_init(void);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_puts(char* str);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);

#endif

// Part 2: Source 
#include "i2c_lcd.h"
#include <string.h>

uint8_t tx_data[4];

void lcd_send_internal(uint8_t rs) {
    // Backlight on (bit 3=1), RW=0 (write)
    tx_data[0] = (data >> 4) | 0x08 | (rs << 0);  // Upper nibble + BL + E=0 RW=0 RS
    tx_data[1] = tx_data[0] | 0x04;               // E=1 pulse
    tx_data[2] = (data & 0x0F) | 0x08 | (rs << 0); // Lower nibble
    tx_data[3] = tx_data[2] | 0x04;               // E=1 pulse

    HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, tx_data, 4, HAL_MAX_DELAY);
    HAL_Delay(2);  // LCD timing
}

void lcd_send_cmd(uint8_t cmd) {
    lcd_send_internal(0);  // RS=0 for command
}

void lcd_send_data(uint8_t data) {
    lcd_send_internal(1);  // RS=1 for data
}

void lcd_init(void) {
    HAL_Delay(50);  // delay after power on so doesnt die

    // Init sequence from HD44780 datasheet (4-bit mode)
    uint8_t init_cmds[] = {0x03, 0x03, 0x03, 0x02};  // function set
    for (int i = 0; i < 4; i++) {
        tx_data[0] = (init_cmds[i] >> 4) | 0x08; // these are the four bytes that are sent to the expansion for 'evaluation'
        tx_data[1] = tx_data[0] | 0x04;
        tx_data[2] = (init_cmds[i] & 0x0F) | 0x08;
        tx_data[3] = tx_data[2] | 0x04;
        HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, tx_data, 4, HAL_MAX_DELAY);
        HAL_Delay(5);
    }

    lcd_send_cmd(0x28);  // 4-bit, 2 lines, 5x8 font
    lcd_send_cmd(0x08);  // Display off
    lcd_send_cmd(0x01);  // Clear
    HAL_Delay(2);
    lcd_send_cmd(0x06);  // Entry mode
    lcd_send_cmd(0x0C);  // Display on, no cursor
}

void lcd_clear(void) {
    lcd_send_cmd(0x01);
    HAL_Delay(2);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    lcd_send_cmd(addr);
}

void lcd_puts(char* str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

main() {
    /* USER CODE BEGIN 2 */
  lcd_init();
  lcd_clear();
  lcd_set_cursor(0, 0);
  lcd_puts("S I D 2 5 8 9 5 2 7 3");
  lcd_set_cursor(1, 0);
  lcd_puts("M E C H A T R O N I C S");
  lcd_set_cursor(1, 11);
  lcd_puts("1");
  /* USER CODE END 2 */
}
