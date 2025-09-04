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
 * @date September 04th 2025
 *
 */

#include "mqtt_device_trigger.h"
#include "mqtt_switch.h"
#include "ha_switch.h"

unsigned HaSwitch::s_m_count = 1;

HaSwitch::HaSwitch(bool gui_switch, user_cb user_callback) : m_user_callback(user_callback),
                                                             m_switch_p(nullptr) {

  if (gui_switch)
    m_switch_p = new MqttSwitch(s_m_count);
  else
    m_switch_p = new MqttDeviceTrigger(s_m_count);
  s_m_count++;
}

HaSwitch::~HaSwitch() {
  if (m_switch_p)
    delete m_switch_p;
}

esp_err_t HaSwitch::Connect() {

  if (m_switch_p)
    return m_switch_p->Connect(this);
  return ESP_FAIL;
}

bool HaSwitch::get() {

  if (m_switch_p)
    return m_switch_p->get();
  return ESP_FAIL;
}

esp_err_t HaSwitch::set() {

  if (m_switch_p)
    return m_switch_p->set(this);
  return ESP_FAIL;
}

esp_err_t HaSwitch::reset() {

  if (m_switch_p)
    return m_switch_p->reset(this);
  return ESP_FAIL;
}

esp_err_t HaSwitch::toggle() {

  if (m_switch_p)
    return m_switch_p->toggle(this);
  return ESP_FAIL;
}

