// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "IMU_Base.hpp"

IMU_Base::~IMU_Base() {}
  IMU_Base::IMU_Base(std::uint8_t i2c_addr, std::uint32_t freq, I2C_Class* i2c)
  : I2C_Device ( i2c_addr, freq, i2c )
  {}
