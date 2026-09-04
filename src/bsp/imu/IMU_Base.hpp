// Copyright (c) M5Stack. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef __M5_IMU_BASE_H__
#define __M5_IMU_BASE_H__

#include "../i2c/I2C_Class.hpp"

class IMU_Base : public I2C_Device
  {
  public:
    enum imu_spec_t
    {
      imu_spec_none  = 0,
      imu_spec_accel = 0b0001,
      imu_spec_gyro  = 0b0010,
      imu_spec_mag   = 0b0100,
    };

    struct point3d_i16_t
    {
      union
      {
        int16_t value[3];
        struct
        {
          int16_t x;
          int16_t y;
          int16_t z;
        };
      } __attribute__((__packed__));
    };

    struct imu_raw_data_t
    {
      union
      {
        int16_t value[10] = { 0, };
        point3d_i16_t sensor[3];
        struct
        {
          point3d_i16_t accel;
          point3d_i16_t gyro;
          point3d_i16_t mag;
          int16_t temp;
        } __attribute__((__packed__));
      };
    };

    struct imu_convert_param_t
    {
      union
      {
        float value[3] = { 8.0f / 32768.0f, 2000.0f / 32768.0f, 10.0f * 4912.0f / 32768.0f };
        struct
        {
          float accel_res;
          float gyro_res;
          float mag_res;
        };
      };
      float temp_res = 1.0f;
      float temp_offset = 0.0f;
    };

    virtual ~IMU_Base();
    IMU_Base(std::uint8_t i2c_addr, std::uint32_t freq = 400000, I2C_Class* i2c = &In_I2C);
    virtual imu_spec_t begin(I2C_Class* i2c = nullptr) = 0;
    virtual imu_spec_t getImuRawData(imu_raw_data_t* data) const = 0; //{ return (imu_spec_t)0; }
    virtual void getConvertParam(imu_convert_param_t* param) const = 0;//{}
    virtual bool getTempAdc(int16_t* adc) const { (void)adc; return false; };
    virtual bool sleep(void) { return false; };


    virtual bool setINTPinActiveLogic(bool level) { (void)level; return false; }
  };

#endif
