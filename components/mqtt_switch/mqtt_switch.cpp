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
 * @file mqtt_switch.h
 *
 * @brief mqtt_switch Class implementation.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date August 04th 2025
 *
 */

#include <cstring>
#include "mqtt_manager.h"
#include "mqtt_switch.h"

static const char *s_t_config  = "homeassistant/switch/franzininho-wifi/%s/config";
static const char *s_t_command = "franzininho-wifi/%s/action";
static const char *s_t_state   = "franzininho-wifi/%s/state";
static const char *s_t_switch  = "{\"name\":\"Franzininho-WiFi %s\",\"availability_topic\":\"franzininho-wifi/status\",\"command_topic\":\"franzininho-wifi/%s/action\",\"state_topic\":\"franzininho-wifi/%s/state\",\"device\":{\"name\":\"franzininho-wifi\",\"identifiers\":[\"615830010\"]},\"platform\":\"switch\"}";
static const char *s_on = "ON";
static const char *s_off = "OFF";

unsigned MqttSwitch::s_m_count = 1;

MqttSwitch::MqttSwitch() : m_state(0), m_index(s_m_count) {
  s_m_count++;
}

esp_err_t MqttSwitch::Connect() {

  esp_err_t rc;
  char switch_buffer[c_switch_size];
  char command_buffer[c_command_size];

  int temp = snprintf(switch_buffer, c_switch_size, s_t_config, s_m_count);
  if (temp > c_switch_size || temp < 0)
    return ESP_FAIL;

  int offset = temp + 1;
  int new_size = c_switch_size - offset;
  temp = snprintf(switch_buffer + offset, new_size, s_t_switch, s_m_count, s_m_count, s_m_count);
  if (temp > new_size || temp < 0)
      return ESP_FAIL;

  temp = snprintf(command_buffer, c_command_size, s_t_command, s_m_count);
  if (temp > c_command_size || temp < 0)
    return ESP_FAIL;

  if ((rc = MqttSubscribe(command_buffer, 0, NULL)))
    return rc;

  rc = MqttPublish(switch_buffer, switch_buffer+offset, 0, 0, 1);
  return rc;
}

esp_err_t MqttSwitch::set() {

  m_state = 1;
  return PublishState();
}

esp_err_t MqttSwitch::reset() {

  m_state = 0;
  return PublishState();
}

esp_err_t MqttSwitch::toggle() {

  m_state = !m_state;
  return PublishState();
}

void MqttSwitch::mCallback(const char *data, int data_len) {

  if (!strncmp(s_on, data, data_len)) {
    set();
  }
  if (!strncmp(s_off, data, data_len)) {
    reset();
  }
}

esp_err_t MqttSwitch::PublishState() {

  char state_buffer[c_state_size];
  const char *state;
  int temp = snprintf(state_buffer, c_state_size, s_t_state, s_m_count);
  if (temp > c_command_size || temp < 0)
    return ESP_FAIL;
  if (m_state)
    state = s_on;
  else
    state = s_off;
  return MqttPublish(state_buffer, state, 0, 0, 1);
}
