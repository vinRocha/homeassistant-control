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
 * @brief mqtt_switch Class.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date August 04th 2025
 *
 */

#pragma once

#include "esp_err.h"

typedef void (*user_cb)();

class MqttSwitch {
public:
  MqttSwitch(bool ha_switch = 0, user_cb user_callback = nullptr);
  bool get();
  esp_err_t set();
  esp_err_t reset();
  esp_err_t toggle();
  esp_err_t Connect();

private:
  static unsigned s_m_count;
  bool m_state;
  bool m_ha_switch;
  const unsigned m_index;
  user_cb m_user_callback;
  static void mCallback(const char *data, int data_len, void *user_ctx);
  esp_err_t PublishState();
};
