/**
 * SPDX-License-Identifier: GPLv2
 *
 * Copyright (C) 2025  Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * @file app_main.c
 *
 * @brief Example of use of ha_switch component.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date July 12th 2025
 *
 */

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"
#include "mqtt_manager.h"
#include "ha_switch.h"
#include "esp_err.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "images.h"

constexpr gpio_num_t c_led_gpio      = (gpio_num_t) 14;
constexpr   uint64_t c_buttons_gpios = ( 1LLU << 7 | 1LLU << 6 | 1LLU << 5 | \
                                         1LLU << 4 | 1LLU << 3 | 1LLU << 2 );

struct app_ctx {
  TaskHandle_t main_task;
  SSD1306_t *ssd1306;
};

static const char *s_TAG = "main_app";
static app_ctx DRAM_ATTR s_app_cfg;

static esp_err_t s_BoardInit();
static void s_DrawBaseGui();
static void s_PlayAnimation(bool state);
static void s_led_cb(HaSwitch *switch_p);

extern "C" void app_main() {

  SSD1306_t display;
  s_app_cfg.ssd1306 = &display;
  s_app_cfg.main_task = xTaskGetCurrentTaskHandle();

  ESP_ERROR_CHECK(s_BoardInit());

  HaSwitch switches[6] = {
    HaSwitch(false),
    HaSwitch(false),
    HaSwitch(false),
    HaSwitch(false),
    HaSwitch(false),
    HaSwitch(true, s_led_cb)
  };

  ESP_ERROR_CHECK(switches[0].Connect());
  ESP_ERROR_CHECK(switches[1].Connect());
  ESP_ERROR_CHECK(switches[2].Connect());
  ESP_ERROR_CHECK(switches[3].Connect());
  ESP_ERROR_CHECK(switches[4].Connect());
  ESP_ERROR_CHECK(switches[5].Connect());
  ESP_ERROR_CHECK(switches[5].reset());

  int selection = 0;
  s_DrawBaseGui();
  ssd1306_display_text(s_app_cfg.ssd1306, selection + 1, " -> ", 4, false);
  while(true) {
    uint32_t input = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    /* If input, process it. */
    if (input) {
      ESP_LOGI(s_TAG, "Received notification. Processing GPIO mask %#.8x.", input);
      ssd1306_display_text(s_app_cfg.ssd1306, selection + 1, "    ", 4, false);
      switch (input) {
        case (1LLU << 7) : //Button UP
          if (selection == 0)
            selection = 5;
          else
            --selection;
          break;
        case (1LLU << 4) : //Button DOWN
          ++selection;
          selection %= 6;
          break;
        case (1LLU << 2) : //Button ENTER
          switches[selection].toggle();
          s_PlayAnimation(switches[selection].get());
          s_DrawBaseGui();
          break;
        default :
        ESP_LOGI(s_TAG, "Unknown function for GPIO mask: %#.8x", input);
      }
      ssd1306_display_text(s_app_cfg.ssd1306, selection + 1, " -> ", 4, false);
      //deboucing....
      vTaskDelay(180 / portTICK_PERIOD_MS);
      ulTaskNotifyTake(pdTRUE, 0);
    }
    else
      ESP_LOGI(s_TAG, "Notification not received. Trying again...");
  }
}

static void IRAM_ATTR s_GpioIsr(void *args) {

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  uint32_t gpio_intr_status = READ_PERI_REG(GPIO_STATUS_REG);  //read status to get interrupt status for GPIO0-31
  gpio_intr_status &= c_buttons_gpios;
  if (gpio_intr_status) //If Button UP, DOWN or ENTER, wake main app task.
    xTaskNotifyFromISR(s_app_cfg.main_task, gpio_intr_status, 
                       eSetValueWithOverwrite, &xHigherPriorityTaskWoken);
  SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status);   //Clear intr for gpio0-gpio31
  gpio_intr_status = READ_PERI_REG(GPIO_STATUS1_REG);          //read status1 to get interrupt status for GPIO32-39
  SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status);  //Clear intr for gpio32-39, eventough we dont expect any.
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static esp_err_t s_InitGpio(void *args) {

  /* Buttons */
  esp_err_t rc;
  gpio_config_t gpio_handle = {
    .pin_bit_mask = c_buttons_gpios,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE
  };

  if ((rc = gpio_config(&gpio_handle)))
    return rc;

  if ((rc = gpio_isr_register(s_GpioIsr, args, ESP_INTR_FLAG_LOWMED|ESP_INTR_FLAG_IRAM|
                                               ESP_INTR_FLAG_EDGE, NULL)))
    return rc;

  /* LED */
  gpio_handle = {
    .pin_bit_mask = 1LLU << c_led_gpio,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };

  return rc = gpio_config(&gpio_handle);
}

static void s_InitSsd1306() {

  i2c_master_init(s_app_cfg.ssd1306, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
  ssd1306_init(s_app_cfg.ssd1306, 128, 64);
  ssd1306_clear_screen(s_app_cfg.ssd1306, false);
  ssd1306_contrast(s_app_cfg.ssd1306, 0x7f);
  ssd1306_bitmaps(s_app_cfg.ssd1306, 0, 0, franzininho_logo, 128, 64, false);
}

void s_DrawBaseGui() {

  ssd1306_clear_screen(s_app_cfg.ssd1306, false);
  ssd1306_display_text(s_app_cfg.ssd1306, 1, "    Switch 1", 12, false);
  ssd1306_display_text(s_app_cfg.ssd1306, 2, "    Switch 2", 12, false);
  ssd1306_display_text(s_app_cfg.ssd1306, 3, "    Switch 3", 12, false);
  ssd1306_display_text(s_app_cfg.ssd1306, 4, "    Switch 4", 12, false);
  ssd1306_display_text(s_app_cfg.ssd1306, 5, "    Switch 5", 12, false);
  ssd1306_display_text(s_app_cfg.ssd1306, 6, "    Red LED ", 12, false);
}

void s_PlayAnimation(bool state) {

  ssd1306_clear_screen(s_app_cfg.ssd1306, false);
  if (state) {
    for (int i = 0; i < 2 * FRAME_COUNT - 10; i++) {
      ssd1306_bitmaps(s_app_cfg.ssd1306, FRAME_START_X, FRAME_START_Y, light_on[i % FRAME_COUNT],
                       FRAME_WIDTH, FRAME_HEIGHT, false);
      vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
    }
  }
  else {
    for (int i = 0; i < 3 * FRAME_COUNT / 2; i++) {
      ssd1306_bitmaps(s_app_cfg.ssd1306, FRAME_START_X, FRAME_START_Y, light_off[i % FRAME_COUNT],
                       FRAME_WIDTH, FRAME_HEIGHT, false);
      vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
    }
  }
}

esp_err_t s_BoardInit() {

  esp_err_t rc;
  s_InitSsd1306();
  if ((rc = nvs_flash_init()))
    return rc;
  if ((rc = esp_netif_init()))
    return rc;
  if ((rc = s_InitGpio(NULL)))
    return rc;
  if ((rc = esp_event_loop_create_default()))
    return rc;
  if ((rc = example_connect()))
    return rc;
  if ((rc = MqttInit()))
    return rc;
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  return rc = MqttPublish("franzininho-wifi/status", "online", 0, 0, 1);
}

void s_led_cb(HaSwitch *switch_p) {

  gpio_set_level(c_led_gpio, switch_p->get());
}
