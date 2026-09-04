#include "PCA9557.hpp"

#ifdef PCA9557_DEBUG
#define DEBUG_PRINTE(s) Serial.print(s)
#define DEBUG_PRINTELN(s) Serial.println(s)
#else
#define DEBUG_PRINTE(s)
#define DEBUG_PRINTELN(s)
#endif

// Ref: datasheet page 17
#define INPUT_PORT_REGISTER         (0x00)
#define OUTPUT_PORT_REGISTER        (0x01)
#define POLARITY_INVERSION_REGISTER (0x02) // Polarity Inversion:       1: Inver Input Logic    0: Non-Inver Input Logic
#define CONFIGURATION_REGISTER      (0x03) // Mode:    1: INPUT   0: OUTPUT

#define CHECK_FAIL_AND_RETURN(a) ({ \
    if (!a) return false; \
})

PCA9557_Class::PCA9557_Class() {
    this->_addr = 0;
    this->_wire = nullptr;
}

PCA9557_Class::PCA9557_Class(int address, I2C_Class *bus) {
    this->_addr = address;
    this->_wire = bus;
}

void PCA9557_Class::begin(int address, I2C_Class *bus) {
    this->_addr = address;
    this->_wire = bus;
}

bool PCA9557_Class::pinMode(int pin, int mode) {
    uint8_t mode_reg_value = 0;
    CHECK_FAIL_AND_RETURN(this->read_register(CONFIGURATION_REGISTER, &mode_reg_value));
    if (mode == INPUT) {
        bitSet(mode_reg_value, pin); // Set => INPUT
    } else if (mode == OUTPUT) {
        bitClear(mode_reg_value, pin); // Clear => OUTPUT
    }
    CHECK_FAIL_AND_RETURN(this->write_register(CONFIGURATION_REGISTER, mode_reg_value));

    if (mode == INPUT) {
        CHECK_FAIL_AND_RETURN(this->write_register(POLARITY_INVERSION_REGISTER, 0x00)); // Away disable polarity inversion
    }

    return true;
}

bool PCA9557_Class::digitalWrite(int pin, int value) {
    uint8_t output_reg_value = 0;
    CHECK_FAIL_AND_RETURN(this->read_register(OUTPUT_PORT_REGISTER, &output_reg_value));
    bitWrite(output_reg_value, pin, value == HIGH ? 1 : 0);
    CHECK_FAIL_AND_RETURN(this->write_register(OUTPUT_PORT_REGISTER, output_reg_value));

    return true;
}

int PCA9557_Class::digitalRead(int pin) {
    uint8_t input_reg_value = 0;
    if (!this->read_register(INPUT_PORT_REGISTER, &input_reg_value)) {
        return LOW;
    }

    return bitRead(input_reg_value, pin) ? HIGH : LOW;
}

bool PCA9557_Class::read_register(uint8_t reg, uint8_t *value) {
    if (!_wire) {
        DEBUG_PRINTELN("PCA9557_Class I2C bus not initialized");
        return false;
    }
    
    *value = _wire->readRegister8(this->_addr, reg, 400000);
    return true;
}

bool PCA9557_Class::write_register(uint8_t reg, uint8_t value) {
    if (!_wire) {
        DEBUG_PRINTELN("PCA9557_Class I2C bus not initialized");
        return false;
    }
    
    return _wire->writeRegister8(this->_addr, reg, value, 400000);
}
