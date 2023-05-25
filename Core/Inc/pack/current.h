/**
 * @file current.h
 * @brief Functions that handle current measurement
 *
 * @date Sep 24, 2021
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */
#ifndef CURRENT_H
#define CURRENT_H

#define MEASURE_SAMPLE_SIZE 128 // Number of samples per measurement

/** @brief Sensor used for current measurement */
typedef enum {
    CURRENT_SENSOR_50 = 0,  // First hall effect sensor (50A)
    CURRENT_SENSOR_300,     // Second hall effect sensor (300A)
    CURRENT_SENSOR_SHUNT,   // Shunt resistor
    CURRENT_SENSOR_NUM
} CURRENT_SENSOR;

typedef float current_t;

/** @brief Start current measurement using DMA */
void current_start_measure();

/**
 * @brief Reads TS current from current sensors
 * 
 * @param shunt_adc_val The value read from the shunt ADC
 * @return uint32_t The timestamp at which the measurement occurred
 */
uint32_t current_read(float shunt_adc_val);

/** @brief Zeroes the Hall-effect sensor */
void current_zero();

/**
 * @brief Returns the current flowing through the TS
 * 
 * @return current_t The current read from the sensors
 */
current_t current_get_current();

/**
 * @brief Returns the currents measured by each current sensor. Refer to CURRENT_SENSOR enum for value-sensor association
 * 
 * @return current_t* A refernce to the array of measures  
 */
current_t * current_get_current_sensors();

/**
 * @brief Returns the current flowing through the specified sensor
 * 
 * @param sensor The sensor to read the current from
 * @return current_t The measured current
 */
current_t current_get_current_from_sensor(CURRENT_SENSOR sensor);

/** @brief Check for current errors */
void current_check_errors();

#endif // CURRENT_H
