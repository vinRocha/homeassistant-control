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
 * @file mqtt_switch.cpp
 *
 * @brief mqtt_switch Class implementation.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date September 04th 2025
 *
 */

#include <cstring>
#include "mqtt_manager.h"
#include "mqtt_switch.h"

static const char *s_t_config  = "homeassistant/switch/franzininho-wifi/%s/config";
static const char *s_config_switch  = "{\"name\":\"Franzininho-WiFi %s\",\"availability_topic\":\"franzininho-wifi/status\",\"command_topic\":\"franzininho-wifi/%s/action\",\"device\":{\"name\":\"franzininho-wifi\",\"identifiers\":[\"615830010\"]},\"platform\":\"switch\",\"state_topic\":\"franzininho-wifi/%s/state\"}";

esp_err_t MqttSwitch::Connect(HaSwitch *ha_switch_p) {

  constexpr int config_size   = 400;
  constexpr int action_size   =  30;
  constexpr int instance_size =   8;

  esp_err_t rc;
  char config_buffer[config_size];
  char action_buffer[action_size];
  char instance[instance_size];

  snprintf(instance, instance_size, "s_%u", m_index);

  int temp, offset, newsize;

  temp = snprintf(config_buffer, config_size, s_t_config, instance);
  if (temp > config_size || temp < 0)
    return ESP_FAIL;

  offset = temp + 1;
  newsize = config_size - offset;
  temp = snprintf(config_buffer + offset, newsize, s_config_switch, instance, instance, instance);
  if (temp > newsize || temp < 0)
      return ESP_FAIL;

  temp = snprintf(action_buffer, action_size, s_t_action, instance);
  if (temp > action_size || temp < 0)
    return ESP_FAIL;


  if ((rc = MqttSubscribe(action_buffer, 0, mCallback, ha_switch_p)))
    return rc;

  return rc = MqttPublish(config_buffer, config_buffer+offset, 0, 0, 1);
}

esp_err_t MqttSwitch::set(HaSwitch *ha_switch_p) {

  m_state = true;
  if (ha_switch_p->m_user_callback)
    ha_switch_p->m_user_callback(ha_switch_p);
  return PublishState();
}

esp_err_t MqttSwitch::reset(HaSwitch *ha_switch_p) {

  m_state = false;
  if (ha_switch_p->m_user_callback)
    ha_switch_p->m_user_callback(ha_switch_p);
  return PublishState();
}

esp_err_t MqttSwitch::PublishState() {

  constexpr int topic_size    =  30;
  constexpr int instance_size =   8;

  char topic_buffer[topic_size];
  const char *state;

  char instance[instance_size];
  snprintf(instance, instance_size, "s_%u", m_index);

  int temp = snprintf(topic_buffer, topic_size, s_t_state, instance);
  if (temp > topic_size || temp < 0)
    return ESP_FAIL;
  if (m_state)
    state = s_on;
  else
    state = s_off;

  return MqttPublish(topic_buffer, state, 0, 0, 1);
}
