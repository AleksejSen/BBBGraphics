/*
 * adc.h
 *
 *  Created on: Oct 28, 2020
 *      Author: alex
 */

#ifndef ADC_H_
#define ADC_H_

/* ANALOG */
#define LDR_PATH "/sys/bus/iio/devices/iio:device0/in_voltage"

int readAnalog(int number);



#endif /* ADC_H_ */
