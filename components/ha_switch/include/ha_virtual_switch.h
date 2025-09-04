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
 * @file ha_virtual_switch.h
 *
 * @brief ha_virtual_switch Interface definition.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date Septmeber 04th 2025
 *
 */

#pragma once

#include "esp_err.h"
#include "ha_switch.h"

class HaVirtualSwitch {
public:
  HaVirtualSwitch(unsigned index) : m_state(0), m_index(index) {}
  virtual ~HaVirtualSwitch() {}
  bool get();
  esp_err_t toggle(HaSwitch *ha_switch_p);
  virtual esp_err_t set(HaSwitch *ha_switch_p) = 0;
  virtual esp_err_t reset(HaSwitch *ha_switch_p) = 0;
  virtual esp_err_t Connect(HaSwitch *ha_switch_p) = 0;

protected:
  bool m_state;
  const unsigned m_index;
  virtual esp_err_t PublishState() = 0;
  static void mCallback(const char *data, int data_len, void *user_ctx);
  static const char *s_t_action;
  static const char *s_t_state;
  static const char *s_on;
  static const char *s_off;
  static const char *s_press;
};
