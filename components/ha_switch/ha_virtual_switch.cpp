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
 * @file ha_virtual_switch.cpp
 *
 * @brief ha_virutal_switch Class implemetation.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date September 04th 2025
 *
 */

#include <cstring>
#include "ha_virtual_switch.h"

const char* HaVirtualSwitch::s_t_action = "franzininho-wifi/%s/action";
const char* HaVirtualSwitch::s_t_state   = "franzininho-wifi/%s/state";
const char* HaVirtualSwitch::s_on = "ON";
const char* HaVirtualSwitch::s_off = "OFF";
const char* HaVirtualSwitch::s_press = "PRESS";

bool HaVirtualSwitch::get() {

  return m_state;
}

esp_err_t HaVirtualSwitch::toggle(HaSwitch *ha_switch_p) {

  m_state = !m_state;
  if (ha_switch_p->m_user_callback)
    ha_switch_p->m_user_callback(ha_switch_p);
  return PublishState();
}

void HaVirtualSwitch::mCallback(const char *data, int data_len, void *user_ctx) {

  if (user_ctx) {
    HaSwitch *ha_switch_p = (HaSwitch*) user_ctx;
    if (!strncmp(s_on, data, data_len)) {
      ha_switch_p->set();
    }
    if (!strncmp(s_off, data, data_len)) {
      ha_switch_p->reset();
    }
    if (ha_switch_p->m_user_callback)
      ha_switch_p->m_user_callback((HaSwitch*)user_ctx);
  }
}
