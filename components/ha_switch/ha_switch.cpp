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
 * @file ha_switch.cpp
 *
 * @brief ha_switch Class implementation.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date August 04th 2025
 *
 */

#include <cstring>
#include "mqtt_manager.h"
#include "mqtt_switch.h"

static const char *s_t_config  = "homeassistant/%s/franzininho-wifi/%s/config";
static const char *s_t_action = "franzininho-wifi/%s/action";
static const char *s_t_state   = "franzininho-wifi/%s/state";
static const char *s_config_switch  = "{\"name\":\"Franzininho-WiFi %s\",\"availability_topic\":\"franzininho-wifi/status\",\"command_topic\":\"franzininho-wifi/%s/action\",\"device\":{\"name\":\"franzininho-wifi\",\"identifiers\":[\"615830010\"]},\"platform\":\"switch\",\"state_topic\":\"franzininho-wifi/%s/state\"}";
static const char *s_config_trigger = "{\"name\":\"Franzininho-WiFi %s\",\"availability_topic\":\"franzininho-wifi/status\",\"topic\":\"franzininho-wifi/%s/action\",\"device\":{\"name\":\"franzininho-wifi\",\"identifiers\":[\"615830010\"]},\"platform\":\"device_automation\",\"automation_type\":\"trigger\",\"type\":\"button_short_press\",\"subtype\":\"button_%u\",\"payload\":\"%s\"}";
static const char *s_switch  = "switch";
static const char *s_trigger = "device_automation";
static const char *s_on = "ON";
static const char *s_off = "OFF";
static const char *s_press = "PRESS";

unsigned MqttSwitch::s_m_count = 1;

MqttSwitch::MqttSwitch(bool ha_switch, user_cb user_callback) : m_state(0),
                                                                m_ha_switch(ha_switch),
                                                                m_index(s_m_count),
                                                                m_user_callback(user_callback) {
  s_m_count++;
}

esp_err_t MqttSwitch::Connect() {

  constexpr int config_size   = 400;
  constexpr int command_size  =  30;
  constexpr int instance_size =   8;

  esp_err_t rc;
  char config_buffer[config_size];
  char command_buffer[command_size];
  char instance[instance_size];

  snprintf(instance, instance_size, "s_%u", m_index);

  int temp, offset, newsize;

  if (m_ha_switch) {
    temp = snprintf(config_buffer, config_size, s_t_config, s_switch, instance);
    if (temp > config_size || temp < 0)
      return ESP_FAIL;

    offset = temp + 1;
    newsize = config_size - offset;
    temp = snprintf(config_buffer + offset, newsize, s_config_switch, instance, instance, instance);
    if (temp > newsize || temp < 0)
        return ESP_FAIL;

    temp = snprintf(command_buffer, command_size, s_t_action, instance);
    if (temp > command_size || temp < 0)
      return ESP_FAIL;
  }

  else {
    temp = snprintf(config_buffer, config_size, s_t_config, s_trigger, instance);
    if (temp > config_size || temp < 0)
      return ESP_FAIL;

    offset = temp + 1;
    newsize = config_size - offset;
    temp = snprintf(config_buffer + offset, newsize, s_config_trigger, instance, instance, m_index, s_press);
    if (temp > newsize || temp < 0)
        return ESP_FAIL;

    temp = snprintf(command_buffer, command_size, s_t_state, instance);
    if (temp > command_size || temp < 0)
      return ESP_FAIL;
  }

  if ((rc = MqttSubscribe(command_buffer, 0, mCallback, this)))
    return rc;

  return rc = MqttPublish(config_buffer, config_buffer+offset, 0, 0, 1);
}

bool MqttSwitch::get() {

  return m_state;
}

esp_err_t MqttSwitch::set() {

  m_state = true;
  if (m_user_callback)
    m_user_callback(this);
  if (m_ha_switch)
    return PublishState();
  return ESP_OK;
}

esp_err_t MqttSwitch::reset() {

  m_state = false;
  if (m_user_callback)
    m_user_callback(this);
  if (m_ha_switch)
    return PublishState();
  return ESP_OK;
}

esp_err_t MqttSwitch::toggle() {

  m_state = !m_state;
  if (m_user_callback)
    m_user_callback(this);
  return PublishState();
}

void MqttSwitch::mCallback(const char *data, int data_len, void *user_ctx) {

  if (user_ctx) {
    MqttSwitch *my_switch = (MqttSwitch*) user_ctx;
    if (!strncmp(s_on, data, data_len)) {
      my_switch->set();
    }
    if (!strncmp(s_off, data, data_len)) {
      my_switch->reset();
    }
    if (my_switch->m_user_callback)
      my_switch->m_user_callback((MqttSwitch*)user_ctx);
  }
}

esp_err_t MqttSwitch::PublishState() {

  constexpr int topic_size    =  30;
  constexpr int instance_size =   8;

  char topic_buffer[topic_size];
  const char *state;
  int retain;

  char instance[instance_size];
  snprintf(instance, instance_size, "s_%u", m_index);

  if (m_ha_switch) {
    retain = 1;
    int temp = snprintf(topic_buffer, topic_size, s_t_state, instance);
    if (temp > topic_size || temp < 0)
      return ESP_FAIL;
    if (m_state)
      state = s_on;
    else
      state = s_off;
  }
  else {
    retain = 0;
    int temp = snprintf(topic_buffer, topic_size, s_t_action, instance);
    if (temp > topic_size || temp < 0)
      return ESP_FAIL;
    state = s_press;
  }

  return MqttPublish(topic_buffer, state, 0, 0, retain);
}
