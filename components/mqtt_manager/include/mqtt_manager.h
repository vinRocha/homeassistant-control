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
 * @file mqtt_manager.h
 *
 * @brief interface to interact with mqtt_manager component.
 *
 * @author Vinicius Silva <silva.viniciusr@gmail.com>
 *
 * @date July 12th 2025
 *
 * Component based on
 * https://github.com/espressif/esp-idf/tree/v5.4.2/examples/protocols/mqtt/ssl
 *
 */

#pragma once

#include "esp_err.h"

esp_err_t MqttInit(void);
esp_err_t MqttPublish(const char *topic, const char *message, int len, int qos, int retain);
esp_err_t MqttSubscribe(const char *topic, int qos);
