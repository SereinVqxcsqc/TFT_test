#include "st7735s.h"

#define TAG "LCD"
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08

static void lcd_send_cmd(spi_device_handle_t spi, const uint8_t cmd) {
    gpio_set_level(PIN_NUM_DC, 0);  // DC low for command
    spi_transaction_t t = {
        .length = 8,
        .tx_buffer = &cmd,
    };
    spi_device_transmit(spi, &t);
}

static void lcd_send_data(spi_device_handle_t spi, const uint8_t *data, int len) {
    if (len == 0) return;
    gpio_set_level(PIN_NUM_DC, 1);  // DC high for data
    spi_transaction_t t = {
        .length = len * 8,
        .tx_buffer = data,
    };
    spi_device_transmit(spi, &t);
}

static void lcd_send_data_byte(spi_device_handle_t spi, uint8_t data) {
    lcd_send_data(spi, &data, 1);
}

void lcd_reset() {
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void lcd_set_addr_window(spi_device_handle_t spi, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += 2; x1 += 2;  // ST7735S 通常偏移 2
    y0 += 1; y1 += 1;  // Y 偏移 1

    uint8_t data[4];
    lcd_send_cmd(spi, 0x2A); // Column addr set
    data[0] = x0 >> 8; data[1] = x0 & 0xFF;
    data[2] = x1 >> 8; data[3] = x1 & 0xFF;
    lcd_send_data(spi, data, 4);

    lcd_send_cmd(spi, 0x2B); // Row addr set
    data[0] = y0 >> 8; data[1] = y0 & 0xFF;
    data[2] = y1 >> 8; data[3] = y1 & 0xFF;
    lcd_send_data(spi, data, 4);

    lcd_send_cmd(spi, 0x2C); // Memory write
}

void lcd_draw_pixel(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return;
    lcd_set_addr_window(spi, x, y, x, y);
    uint8_t data[] = { color >> 8, color & 0xFF };
    lcd_send_data(spi, data, 2);
}

void lcd_draw_bitmap(spi_device_handle_t spi, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap) {
}

void lcd_fill_screen(spi_device_handle_t spi, uint16_t color) {
    lcd_set_addr_window(spi, 0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    gpio_set_level(PIN_NUM_DC, 1);
    spi_transaction_t t = {
        .length = LCD_WIDTH * LCD_HEIGHT * 16,
        .flags = SPI_TRANS_USE_TXDATA,
    };
    for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        spi_transaction_t trans = {
            .length = 16,
            .tx_data = { data[0], data[1] },
            .flags = SPI_TRANS_USE_TXDATA,
        };
        spi_device_transmit(spi, &trans);
    }
}

void lcd_init(spi_device_handle_t *spi_out) {
    esp_err_t ret;

    // 初始化 GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_NUM_DC) | (1ULL << PIN_NUM_RST) | (1ULL << PIN_NUM_BLK),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_BLK, 1);

    // 初始化 SPI 总线
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 128 * 128 * 2 + 8
    };
    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000, // 26 MHz
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = NULL,
    };
    spi_device_handle_t spi;
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
    *spi_out = spi;

    // 硬件复位
    lcd_reset();

    // 初始化指令序列（ST7735S）
    lcd_send_cmd(spi, 0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));

    lcd_send_cmd(spi, 0x3A); // Interface Pixel Format
    lcd_send_data_byte(spi, 0x05); // 16-bit/pixel

    lcd_send_cmd(spi, 0x36); // Memory Data Access Control
    lcd_send_data_byte(spi, 0x00 | ST7735_MADCTL_BGR);  // RGB/BGR设置

    lcd_send_cmd(spi, 0x29); // Display On
    vTaskDelay(pdMS_TO_TICKS(20));

    ESP_LOGI(TAG, "ST7735S LCD initialized");
}
