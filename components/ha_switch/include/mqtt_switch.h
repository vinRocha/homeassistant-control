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
 * @brief mqtt_switch Interface definition.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date Septmeber 04th 2025
 *
 */

#pragma once

#include "ha_virtual_switch.h"

class MqttSwitch : public HaVirtualSwitch {
public:
  MqttSwitch(unsigned index) : HaVirtualSwitch(index) {}
  ~MqttSwitch() override {}
  esp_err_t set(HaSwitch *ha_switch_p) override;
  esp_err_t reset(HaSwitch *ha_switch_p) override;
  esp_err_t Connect(HaSwitch *ha_switch_p) override;

private:
  esp_err_t PublishState() override;
};
