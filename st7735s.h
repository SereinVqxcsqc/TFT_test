#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 引脚配置（请根据你的连接方式调整）
#define PIN_NUM_MISO  -1 // MISO 不连接
#define PIN_NUM_MOSI  11
#define PIN_NUM_CLK   12
#define PIN_NUM_CS    10
#define PIN_NUM_DC    46
#define PIN_NUM_RST   9
#define PIN_NUM_BLK   2  // 背光（可选）

#define LCD_WIDTH     128
#define LCD_HEIGHT    128

void lcd_init(spi_device_handle_t *spi);
void lcd_draw_pixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color);
void lcd_fill_screen(spi_device_handle_t spi, uint16_t color);
// void lcd_draw_bitmap(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap);
