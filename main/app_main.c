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
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "driver/gpio.h"
#include "mqtt_manager.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "images.h"
#include "oled_driver.h"

#define BUTTON_GPIO          (3)

#define HA_CONFIG_MSG        "{\"automation_type\":\"trigger\",\"topic\":\"franzininho-wifi/button_1/trigger\",\"payload\":\"pressed\",\"type\":\"button_short_press\",\"subtype\":\"button_1\",\"device\":{\"name\":\"franzininho-wifi\",\"identifiers\":[\"615830010\"]},\"name\":\"Franzininho-WiFi Button 1\",\"platform\":\"device_automation\"}"
#define HA_CONFIG_TOPIC      "homeassistant/device_automation/615830010/button_1/config"
#define HA_COMMAND_TOPIC     "franzininho-wifi/button_1/trigger"

static const char *s_TAG = "main_app";
static TaskHandle_t s_app_task;

static void s_GpioIsr(void *args);
static esp_err_t s_InitGpio(void);

void app_main(void) {

  s_app_task = xTaskGetCurrentTaskHandle();
  bool state = false;
  int64_t previous_time = esp_timer_get_time();

  ESP_LOGI(s_TAG, "[APP] Startup..");
  ESP_LOGI(s_TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
  ESP_LOGI(s_TAG, "[APP] IDF version: %s", esp_get_idf_version());
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(OledInit());
  OledDrawBitmap(128, 64, 0, 0, franzininho_logo);
  ESP_ERROR_CHECK(example_connect());
  ESP_ERROR_CHECK(s_InitGpio());
  ESP_ERROR_CHECK(MqttInit());

  ESP_LOGI(s_TAG, "[APP] Free memory after initialization: %" PRIu32 " bytes", esp_get_free_heap_size());

  vTaskDelay(pdMS_TO_TICKS(5000));

  ESP_LOGI(s_TAG, "Registering HA MQTT Button Device");
  ESP_ERROR_CHECK(MqttPublish(HA_CONFIG_TOPIC, HA_CONFIG_MSG, 0, 0, 1));

  while(true) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if ((esp_timer_get_time() - previous_time) > 1000 * 1000) {
      if (state) {
        for (int i = 0; i < 3 * FRAME_COUNT / 2; i++) {
          OledDrawBitmap(FRAME_WIDTH, FRAME_HEIGHT, FRAME_START_X, FRAME_START_Y,
                         light_off[i % FRAME_COUNT]);
          vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
        }
      }
      else {
        for (int i = 0; i < 2 * FRAME_COUNT - 10; i++) {
          OledDrawBitmap(FRAME_WIDTH, FRAME_HEIGHT, FRAME_START_X, FRAME_START_Y,
                         light_on[i % FRAME_COUNT]);
          vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
        }
      }
      MqttPublish(HA_COMMAND_TOPIC, "pressed", 0, 0, 0);
      state = !state;
      previous_time = esp_timer_get_time();
    }
  }
}

esp_err_t s_InitGpio(void) {

  esp_err_t rc;
  const  gpio_config_t gpio_handle = {
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

  rc = gpio_isr_handler_add(BUTTON_GPIO, s_GpioIsr, NULL);
  return rc;
}

void s_GpioIsr(void *args) {

  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  vTaskNotifyGiveFromISR(s_app_task, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
