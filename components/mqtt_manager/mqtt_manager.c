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
 * @file mqtt_manager.c
 *
 * @brief Encapsulation of esp-idf mqtt_client library.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date July 12th 2025
 *
 * Component based on
 * https://github.com/espressif/esp-idf/tree/v5.4.2/examples/protocols/mqtt/ssl
 *
 */

#include <stdint.h>
#include <string.h>
#include <sys/param.h>
#include "esp_err.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "mqtt_manager.h"

#ifdef CONFIG_MQTT_NULL_CLIENT_ID
#define MQTT_NULL_CLIENT_ID true
#else
#define MQTT_NULL_CLIENT_ID false
#endif

static const char *s_TAG = "MQTT_M";

typedef struct subscriptions {
  char *topic;
  mqtt_subscription_cb callback;
  void *user_ctx;
  struct subscriptions *next;
} subscriptions;

struct driver_state {

/* Is driver initialised? */
  bool initialised;

/* MQTT client handle */
  esp_mqtt_client_handle_t client;

/* MQTT subscriptions */
  subscriptions *head;
  subscriptions *tail;

/* Error check variable */
  esp_err_t rc;
};
static struct driver_state s_d_state = {0};

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void s_MqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

  ESP_LOGD(s_TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  subscriptions *current;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_BEFORE_CONNECT:
    ESP_LOGI(s_TAG, "MQTT_EVENT_BEFORE_CONNECT");
    break;

  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(s_TAG, "MQTT_EVENT_CONNECTED");
    break;

  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(s_TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(s_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(s_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(s_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_DATA:
    ESP_LOGI(s_TAG, "MQTT_EVENT_DATA");
    current = s_d_state.head;
    while (current != s_d_state.tail) {
        if (!strncmp(event->topic, current->topic, event->topic_len)) {
          if (current->callback)
            current->callback(event->data, event->data_len, current->user_ctx);
          break;
        }
        current = current->next;
    }
    break;

  case MQTT_EVENT_ERROR:
    ESP_LOGI(s_TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      ESP_LOGI(s_TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
      ESP_LOGI(s_TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
      ESP_LOGI(s_TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
               strerror(event->error_handle->esp_transport_sock_errno));
    } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
      ESP_LOGI(s_TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
    } else {
      ESP_LOGW(s_TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
    }
    break;

  default:
    ESP_LOGI(s_TAG, "Other event id:%d", event->event_id);
    break;
  }
}

esp_err_t MqttInit(void) {

  if (s_d_state.initialised) {
    return s_d_state.rc = ESP_ERR_INVALID_STATE;
  }

  const esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {
      .address.uri = CONFIG_MQTT_BROKER_URI,
      .verification.crt_bundle_attach = esp_crt_bundle_attach
    },
    .credentials = {
      .username = CONFIG_MQTT_USERNAME,
      .authentication.password = CONFIG_MQTT_PASSWORD,
      .set_null_client_id = MQTT_NULL_CLIENT_ID
    }
  };

  s_d_state.client = esp_mqtt_client_init(&mqtt_cfg);
  if (!s_d_state.client) {
    return s_d_state.rc = ESP_FAIL;
  }

  /* The last argument may be used to pass data to the event handler, in this example s_MqttEventHandler */
  s_d_state.rc = esp_mqtt_client_register_event(s_d_state.client, ESP_EVENT_ANY_ID, s_MqttEventHandler, NULL);
  if (s_d_state.rc)
    return s_d_state.rc;

  s_d_state.rc = esp_mqtt_client_start(s_d_state.client);
  if (s_d_state.rc)
    return s_d_state.rc;

  s_d_state.head = (subscriptions*) calloc(1, sizeof(subscriptions));
  if (!s_d_state.head)
    return ESP_ERR_NO_MEM;

  s_d_state.tail = s_d_state.head;
  s_d_state.initialised = true;
  return ESP_OK;
}

esp_err_t MqttPublish(const char *topic, const char *message, int len, int qos, int retain) {

  if (!s_d_state.initialised)
    return ESP_ERR_INVALID_STATE;

  esp_err_t rc = esp_mqtt_client_publish(s_d_state.client, topic, message, len, qos, retain);
  if (rc < 0)
    return ESP_FAIL;
  return ESP_OK;
}

esp_err_t MqttSubscribe(const char *topic, int qos, mqtt_subscription_cb callback, void *user_ctx) {

  if (!s_d_state.initialised)
    return ESP_ERR_INVALID_STATE;

  if (!s_d_state.tail)
    return ESP_ERR_NO_MEM;

  unsigned topic_len = strlen(topic);
  if (topic_len > CONFIG_MQTT_SUB_TOPIC_MAX_LEN)
    return ESP_ERR_INVALID_ARG;

  topic_len++;
  s_d_state.tail->topic = (char*) malloc(topic_len);

  if (!s_d_state.tail->topic)
    return ESP_ERR_NO_MEM;

  if (esp_mqtt_client_subscribe(s_d_state.client, topic, qos) < 0) {
    free (s_d_state.tail->topic);
    return ESP_FAIL;
  }

  strncpy(s_d_state.tail->topic, topic, topic_len);
  s_d_state.tail->callback = callback;
  s_d_state.tail->user_ctx = user_ctx;
  s_d_state.tail->next = (subscriptions*) calloc(1, sizeof(subscriptions));
  s_d_state.tail = s_d_state.tail->next;
  return ESP_OK;
}
