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
 * @file mqtt_device_trigger.cpp
 *
 * @brief mqtt_device_trigger Class implementation.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date September 04th 2025
 *
 */

#include <cstring>
#include "mqtt_manager.h"
#include "mqtt_device_trigger.h"

static const char *s_t_config  = "homeassistant/device_automation/franzininho-wifi/%s/config";
static const char *s_config_trigger = "{\"name\":\"Franzininho-WiFi %s\",\"availability_topic\":\"franzininho-wifi/status\",\"topic\":\"franzininho-wifi/%s/action\",\"device\":{\"name\":\"franzininho-wifi\",\"identifiers\":[\"615830010\"]},\"platform\":\"device_automation\",\"automation_type\":\"trigger\",\"type\":\"button_short_press\",\"subtype\":\"button_%u\",\"payload\":\"%s\"}";

esp_err_t MqttDeviceTrigger::Connect(HaSwitch *ha_switch_p) {

  constexpr int config_size   = 400;
  constexpr int state_size    =  30;
  constexpr int instance_size =   8;

  esp_err_t rc;
  char config_buffer[config_size];
  char state_buffer[state_size];
  char instance[instance_size];

  snprintf(instance, instance_size, "s_%u", m_index);

  int temp, offset, newsize;

  temp = snprintf(config_buffer, config_size, s_t_config, instance);
  if (temp > config_size || temp < 0)
    return ESP_FAIL;

  offset = temp + 1;
  newsize = config_size - offset;
  temp = snprintf(config_buffer + offset, newsize, s_config_trigger, instance, instance, m_index, s_press);
  if (temp > newsize || temp < 0)
      return ESP_FAIL;

  temp = snprintf(state_buffer, state_size, s_t_state, instance);
  if (temp > state_size || temp < 0)
    return ESP_FAIL;

  if ((rc = MqttSubscribe(state_buffer, 0, mCallback, ha_switch_p)))
    return rc;

  return rc = MqttPublish(config_buffer, config_buffer+offset, 0, 0, 1);
}

esp_err_t MqttDeviceTrigger::set(HaSwitch* ha_switch_p) {

  m_state = true;
  if (ha_switch_p->m_user_callback)
    ha_switch_p->m_user_callback(ha_switch_p);
  return ESP_OK;
}

esp_err_t MqttDeviceTrigger::reset(HaSwitch* ha_switch_p) {

  m_state = false;
  if (ha_switch_p->m_user_callback)
    ha_switch_p->m_user_callback(ha_switch_p);
  return ESP_OK;
}

esp_err_t MqttDeviceTrigger::PublishState() {

  constexpr int topic_size    =  30;
  constexpr int instance_size =   8;

  char topic_buffer[topic_size];
  char instance[instance_size];
  snprintf(instance, instance_size, "s_%u", m_index);

  int temp = snprintf(topic_buffer, topic_size, s_t_action, instance);
  if (temp > topic_size || temp < 0)
    return ESP_FAIL;

  return MqttPublish(topic_buffer, s_press, 0, 0, 0);
}
