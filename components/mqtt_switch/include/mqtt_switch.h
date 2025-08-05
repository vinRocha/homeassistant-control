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

class MqttSwitch {
public:
  MqttSwitch();
  esp_err_t Connect();
  esp_err_t set();
  esp_err_t reset();
  esp_err_t toggle();

private:
  static const int c_switch_size  = 400;
  static const int c_command_size =  30;
  static const int c_state_size   =  30;
  static unsigned s_m_count;
  unsigned char m_state;
  const unsigned m_index;
  void mCallback(const char *data, int data_len);
  esp_err_t PublishState();
};
