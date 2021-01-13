/*_____________________________________________________________________________
 │                                                                            |
 │ COPYRIGHT (C) 2021 Mihai Baneu                                             |
 │                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 13.Jan.2021  |
 |                                                                            |
 |___________________________________________________________________________*/

#include "stm32f4xx.h"
#include "stm32rtos.h"
#include "task.h"
#include "queue.h"
#include "system.h"
#include "gpio.h"
#include "adc.h"
#include "ssd1339.h"
#include "printf.h"

typedef struct adc_event_t {
    uint16_t digital_value;
} adc_event_t;

extern const uint8_t u8x8_font_courB18_2x3_r[];

/* Queue used to send and receive complete struct AMessage structures. */
QueueHandle_t adc_queue = NULL;

static void vTaskLED(void *pvParameters)
{
    (void)pvParameters;

    /* led OFF */
    gpio_set_blue_led();

    for (;;) {
        gpio_reset_blue_led();
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        gpio_set_blue_led();
        vTaskDelay(100 / portTICK_PERIOD_MS);

        gpio_reset_blue_led();
        vTaskDelay(100 / portTICK_PERIOD_MS);

        gpio_set_blue_led();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static void vTaskDisplay(void *pvParameters)
{
    adc_event_t adc_event;
    (void)pvParameters;

    ssd1339_hw_control_t hw = {
        .config_control_out  = gpio_config_control_out,
        .config_data_out     = gpio_config_data_out,
        .config_data_in      = gpio_config_data_in,
        .cs_high             = gpio_cs_high,
        .cs_low              = gpio_cs_low,
        .res_high            = gpio_res_high,
        .res_low             = gpio_res_low,
        .dc_high             = gpio_dc_high,
        .dc_low              = gpio_dc_low,
        .wr_high             = gpio_wr_high,
        .wr_low              = gpio_wr_low,
        .rd_high             = gpio_rd_high,
        .rd_low              = gpio_rd_low,
        .cs_wr_high          = gpio_cs_wr_high,
        .cs_wr_low           = gpio_cs_wr_low,
        .cs_rd_high          = gpio_cs_rd_high,
        .cs_rd_low           = gpio_cs_rd_low,
        .dc_cs_wr_high       = gpio_dc_cs_wr_high,
        .dc_cs_wr_low        = gpio_dc_cs_wr_low,
        .data_wr             = gpio_data_wr,
        .data_rd             = gpio_data_rd
    };

    ssd1339_init(hw);
    ssd1339_set_column_address(0, SSD1339_128_COLS-1);
    ssd1339_set_row_address(0, SSD1339_128_ROWS-1);
    ssd1339_set_map_and_color_depth(SSD1339_REMAP_MODE_ODD_EVEN | SSD1339_COLOR_MODE_65K);
    ssd1339_set_use_buildin_lut();
    ssd1339_set_sleep_mode(SSD1339_SLEEP_OFF);
    ssd1339_set_display_mode(SSD1339_MODE_ALL_OFF);
    ssd1339_set_contrast_curent(1,200,200);
    ssd1339_set_precharge_voltage(2, 255, 255);
    ssd1339_set_display_mode(SSD1339_MODE_RESET_TO_NORMAL_DISPLAY);
    ssd1339_draw_clear(0, 0, SSD1339_128_COLS-1, SSD1339_128_COLS-1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    for (;;) {
        if (xQueueReceive(adc_queue, &adc_event, portMAX_DELAY) == pdPASS) {
            char txt[16] = {0};
            float voltage = adc_event.digital_value * 3.3f / 4096.0f;

            sprintf(txt, "%4.2f V", voltage);
            ssd1339_draw_string(u8x8_font_courB18_2x3_r, 20, 50, SSD1339_WHITE, SSD1339_BLACK, txt);
        }
    }
}

static void vTaskAdc(void *pvParameters)
{
    adc_event_t adc_event;
    (void)pvParameters;

    for (;;) {
        adc_event.digital_value = adc_analog_read();
        xQueueSend(adc_queue, &adc_event, (TickType_t) 0);
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

int main(void)
{
    /* initialize the system */
    system_init();

    /* initialize the gpio */
    gpio_init();

    /* initialize the adc */
    adc_init();

    /* create the queues */
    adc_queue = xQueueCreate(1, sizeof(adc_event_t));

    /* create the tasks specific to this application. */
    xTaskCreate(vTaskLED, "vTaskLED", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vTaskDisplay, "vTaskDisplay", configMINIMAL_STACK_SIZE*2, NULL, 2, NULL);
    xTaskCreate(vTaskAdc, "vTaskAdc", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    /* start the scheduler. */
    vTaskStartScheduler();

    /* should never get here ... */
    blink(10);
    return 0;
}
