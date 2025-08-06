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
 * @brief Example of use of mqtt_manager component.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date July 12th 2025
 *
 * Component based on
 * https://github.com/espressif/esp-idf/tree/v5.4.2/examples/protocols/mqtt/ssl
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "driver/gpio.h"
#include "mqtt_manager.h"
#include "mqtt_switch.h"
#include "esp_timer.h"
#include "esp_err.h"
//#include "esp_log.h"
#include "images.h"
#include "oled_driver.h"

#define BUTTON_GPIO    ((gpio_num_t)  3)
#define    LED_GPIO    ((gpio_num_t) 14)

static const char *s_TAG = "main_app";
static TaskHandle_t s_app_task;

static void led_sync_cb();
static MqttSwitch switch_1 (false);
static MqttSwitch switch_2(true, led_sync_cb);

static esp_err_t Init();
static esp_err_t s_InitGpio();
static void s_GpioIsr(void *args);

extern "C" void app_main() {

  s_app_task = xTaskGetCurrentTaskHandle();
  int64_t previous_time = esp_timer_get_time();

  ESP_ERROR_CHECK(Init());

  ESP_ERROR_CHECK(switch_1.Connect());
  ESP_ERROR_CHECK(switch_2.Connect());
  ESP_ERROR_CHECK(switch_2.reset());

  while(true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if ((esp_timer_get_time() - previous_time) > 1000 * 1000) {
      switch_1.toggle();
      if (switch_1.get()) {
        for (int i = 0; i < 2 * FRAME_COUNT - 10; i++) {
          OledDrawBitmap(FRAME_WIDTH, FRAME_HEIGHT, FRAME_START_X, FRAME_START_Y,
                         light_on[i % FRAME_COUNT]);
          vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
        }
      }
      else {
        for (int i = 0; i < 3 * FRAME_COUNT / 2; i++) {
          OledDrawBitmap(FRAME_WIDTH, FRAME_HEIGHT, FRAME_START_X, FRAME_START_Y,
                         light_off[i % FRAME_COUNT]);
          vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
        }
      }
      previous_time = esp_timer_get_time();
    }
  }
}

esp_err_t Init() {

  esp_err_t rc;
  if ((rc = OledInit()))
    return rc;
  if ((rc = OledDrawBitmap(128, 64, 0, 0, franzininho_logo)))
    return rc;
  if ((rc = nvs_flash_init()))
    return rc;
  if ((rc = esp_netif_init()))
    return rc;
  if ((rc = s_InitGpio()))
    return rc;
  if ((rc = esp_event_loop_create_default()))
    return rc;
  if ((rc = example_connect()))
    return rc;
  if ((rc = MqttInit()))
    return rc;
  vTaskDelay(pdMS_TO_TICKS(5000));
  return rc = MqttPublish("franzininho-wifi/status", "online", 0, 0, 1);
}

esp_err_t s_InitGpio(void) {

  esp_err_t rc;
  gpio_config_t gpio_handle = {
    .pin_bit_mask = 1LLU << BUTTON_GPIO,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE
  };

  if ((rc = gpio_config(&gpio_handle)))
    return rc;

  if ((rc = gpio_install_isr_service(ESP_INTR_FLAG_LOWMED|ESP_INTR_FLAG_EDGE)))
    return rc;

  if ((rc = gpio_isr_handler_add(BUTTON_GPIO, s_GpioIsr, NULL)))
    return rc;

  gpio_handle = {
    .pin_bit_mask = 1LLU << LED_GPIO,
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };

  return rc = gpio_config(&gpio_handle);
}

void s_GpioIsr(void *args) {

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(s_app_task, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_sync_cb() {
  gpio_set_level(LED_GPIO, switch_2.get());
}
