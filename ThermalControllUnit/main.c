/*
 * ThermalControllUnit.c
 *
 * Created: 03.11.2023 23:47:23
 * Author : Vanya
 */ 

#include "config.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


#include "string.h"
#include "stdbool.h"
#include "uart_hal.h"
#include "gpio_driver.h"
#include "twi_hal.h"
#include "adc_hal.h"
#include "spi_hall.h"
#include "rtc.h"
#include "stdint.h"
#include <stdio.h>

static FILE mystdout = FDEV_SETUP_STREAM((void *)uart_send_byte, NULL, _FDEV_SETUP_WRITE);

gpio ldA = {(uint8_t *)&PORTD , PORTD4};
gpio ldB = {(uint8_t *)&PORTD , PORTD5};
gpio ld1 = {(uint8_t *)&PORTG , PORTG1};
gpio ld2 = {(uint8_t *)&PORTG , PORTG0};
gpio ioEn = {(uint8_t *)&PORTG , PORTG2};

gpio out1 = {(uint8_t *)&PORTC , PORTC0};
gpio out2 = {(uint8_t *)&PORTC , PORTC1};
gpio out3 = {(uint8_t *)&PORTC , PORTC2};
gpio out4 = {(uint8_t *)&PORTC , PORTC3};
gpio out5 = {(uint8_t *)&PORTC , PORTC4};
gpio out6 = {(uint8_t *)&PORTC , PORTC5};
gpio out7 = {(uint8_t *)&PORTC , PORTC6};
gpio out8 = {(uint8_t *)&PORTC , PORTC7};

gpio fan3_pe5 = {(uint8_t *)&PORTE , PORTE5};
	
rtc_date sys_rtc = {
	.date = 27,
	.month = 10,
	.year = 23,
	.dayofweek = 6,
	.hour = 16,
	.minute = 52,
	.second = 00
};	
	
uint8_t timerIRQ = 0;
uint16_t tempZone1 = 0;
uint16_t tempZone2 = 0;
uint16_t tempZone3 = 0;
	
	
#define	MAX_TEMP	300	
#define	MIN_PWM		25
#define MAX_PWM		250

	
/*
Next - 
ADC configuration					<< DONE
ADC value to Temperature			<< DONE
PWM									<< DONE
FAN control logic (Perhaps - PID)

*/
	
ISR(TIMER1_COMPA_vect){
	timerIRQ = 1;
}

	
int main(void)
{
    uart_init(250000,1);
    twi_init(400000);
	
	DDRC = 0xFF; //Set portC as output
	PORTC = 0xFF; //Set to HW driver to off state
	
    gpio_set_pin_direction(&ldA , PORT_DIR_OUT); gpio_set_pin_level(&ldA, true);
    gpio_set_pin_direction(&ldB , PORT_DIR_OUT); gpio_set_pin_level(&ldB, true);
    gpio_set_pin_direction(&ld1 , PORT_DIR_OUT); gpio_set_pin_level(&ld1, true);
    gpio_set_pin_direction(&ld2 , PORT_DIR_OUT); gpio_set_pin_level(&ld2, true);
	gpio_set_pin_direction(&ioEn , PORT_DIR_OUT); gpio_set_pin_level(&ioEn, true);
    gpio_set_pin_direction(&fan3_pe5 , PORT_DIR_OUT);
	
	sei();
	stdout = &mystdout;
	rtc_set(&sys_rtc);
	//rtc_int_enable(&sys_rtc ,0);
	adc_init();
	spi_init();
	
	//TIMER3 PWM init
	#define  FAN3	OCR3C	
	TCCR3A |= (0b10 << COM3C0) | (0b01 << WGM10);
	TCCR3B = (0b00 << WGM12) | (0b010 << CS10); 
	TCCR3C = (1 << FOC3C);
	FAN3 = 40;
	//Write data to OCR3C
	
	
	
	
	
	//Interrupt controller 
	TIMSK |= (1 << OCIE1A);
	TCCR1A |= (0b10 << WGM00);
	OCR1A = 50; // TOP value
	TCCR1B |= (0b011<< CS00);
	//
	
	
    while (1) 
    {
		
		if(timerIRQ){
			timerIRQ = 0;
			gpio_toggle_pin_level(&out1);
			tempZone2 = getNTC(NTC2);
			if(tempZone2 > MAX_TEMP){
				FAN3 < MAX_PWM ? FAN3++ : NULL;
			}else{
				FAN3 > MIN_PWM ? FAN3-- : NULL;
			}
			
			
			rtc_sync(&sys_rtc);
			printf("Time: %02d-%02d-20%02d; %02d:%02d:%02d; NTC: %03d; FAN:  %03d\r\n", sys_rtc.date, sys_rtc.month, sys_rtc.year, sys_rtc.hour, sys_rtc.minute, sys_rtc.second, tempZone2, FAN3);
			
		}
		
		
		
		
    }
}

