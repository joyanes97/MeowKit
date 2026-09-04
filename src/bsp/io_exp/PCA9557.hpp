#pragma once
#include <Arduino.h>
#include "../i2c/I2C_Class.hpp"

class PCA9557_Class {
    private:
        I2C_Class *_wire;
        uint8_t _addr;

        bool read_register(uint8_t reg, uint8_t *value) ;
        bool write_register(uint8_t reg, uint8_t value) ;

    public:
        PCA9557_Class() ; // Default constructor
        PCA9557_Class(int address, I2C_Class *bus) ; // Constructor with parameters
        
        void begin(int address, I2C_Class *bus); // Initialize function

        bool pinMode(int pin, int mode) ;
        bool digitalWrite(int pin, int value) ;
        int digitalRead(int pin) ;

};
