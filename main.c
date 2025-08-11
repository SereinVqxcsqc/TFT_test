#include <stdio.h>
#include "st7735s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "led_strip.h"
#include "esp_random.h"
#include "lvgl.h"


static const char *TAG = "ST7735S";
#define BLINK_GPIO 35
static uint8_t s_led_state = 0;
 
// #ifdef CONFIG_BLINK_LED_STRIP
static led_strip_handle_t led_strip;
static void blink_RGB(void)
{
    if (s_led_state) {
      // 0~255
        led_strip_set_pixel(led_strip, 0, esp_random() % 256, esp_random() % 256, esp_random() % 256);
        led_strip_refresh(led_strip);
    } else {
        led_strip_clear(led_strip);
    }
}
static void configure_rgb(void)
{
  ESP_LOGI(TAG, "Configuring to blink addressable LED!\r\n");
  led_strip_config_t strip_config = {
    .strip_gpio_num = 48, // GPIO48
    .max_leds = 1,
  };

  // Use RMT driver to control the LED strip
  led_strip_rmt_config_t rmt_config = {
    .resolution_hz = 20 * 1000 * 1000, //20MHz
    .flags.with_dma = false, // Don`t use DMA
  };
  ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
}


// #elif CONFIG_BLINK_LED_GPIO
static void blink_led(void)
{
    gpio_set_level(BLINK_GPIO, s_led_state);
    vTaskDelay(500 / portTICK_PERIOD_MS);
}
static void configure_led(void)
{
  ESP_LOGI(TAG, "Configuring to blink GPIO LED!\r\n");
  gpio_reset_pin(BLINK_GPIO);
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


void app_main(void)
{
  spi_device_handle_t spi;
  lcd_init(&spi);
  configure_led();
  configure_rgb();

  lcd_fill_screen(spi, 0x00000);
  lcd_draw_pixel(spi, 64, 64, 0xF800);
  lcd_draw_pixel(spi, 96, 96, 0xFF00);
  lcd_draw_pixel(spi, 64, 96, 0xF0FF);

  for(;;)
  {
    ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
    blink_led();
    blink_RGB();

    s_led_state = !s_led_state;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}