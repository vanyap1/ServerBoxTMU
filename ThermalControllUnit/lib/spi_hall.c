
/*
 * spi_hall.c
 *
 * Created: 08.05.2023 19:17:36
 *  Author: User
 */ 
#include "spi_hall.h"
#include "gpio_driver.h"
#include <util/delay.h>




//volatile uint8_t spi_rx_data = 0;
volatile uint8_t spi_txrx_done = 0;

gpio spi_tx_pin = {(uint8_t *)&PORTB , PORTB2};
gpio spi_rx_pin = {(uint8_t *)&PORTB , PORTB3};
gpio spi_clk_pin = {(uint8_t *)&PORTB , PORTB1};
gpio spi_ss_pin = {(uint8_t *)&PORTB , PORTB0};




ISR(SPI1_STC_vect){
	if(SPSR & 0x40){
		//spi_rx_data = 0;
		spi_txrx_done = 1;
		}else{
		spi_txrx_done = 1;
	}
}
	
	
void spi_init(){
	gpio_set_pin_direction(&spi_tx_pin , PORT_DIR_OUT);
	gpio_set_pin_level(&spi_tx_pin, true);
	gpio_set_pin_direction(&spi_ss_pin , PORT_DIR_OUT);
	gpio_set_pin_level(&spi_ss_pin, true);
	
	gpio_set_pin_direction(&spi_clk_pin , PORT_DIR_OUT);
	gpio_set_pin_level(&spi_clk_pin, false);
	//DDRC |= (1 << DDC1);
	//DDRE |= (1 << DDE3);	
	SPCR |= (1 << SPIE) | (1 << SPE) | (0 << DORD) | (1 << MSTR)  | (0 << CPOL) | (0 << CPHA) | (0b00 << SPR0);
	SPCR |= (0 << MSTR);
	//Bit 7 – SPIE1: SPI1 Interrupt Enable
	//Bit 6 – SPE1: SPI1 Enable
	//Bit 5 – DORD1: Data1 Order
	//Bit 4 – MSTR1: Master/Slave1 Select
	//Bit 3 – CPOL1: Clock1 Polarity
	//Bit 2 – CPHA1: Clock1 Phase
	//Bits 1:0 – SPR1n: SPI1 Clock Rate Select n [n = 1:0]
	//
	
	SPSR |= (1 << SPI2X);
}

void soft_spi_init(){
	gpio_set_pin_direction(&spi_tx_pin , PORT_DIR_OUT);
	gpio_set_pin_level(&spi_tx_pin, true);
	
	gpio_set_pin_direction(&spi_rx_pin , PORT_DIR_IN);
	gpio_set_pin_level(&spi_rx_pin, true);
	
	gpio_set_pin_direction(&spi_clk_pin , PORT_DIR_OUT);
	gpio_set_pin_level(&spi_clk_pin, false);
	
	gpio_set_pin_direction(&spi_ss_pin , PORT_DIR_OUT);
	gpio_set_pin_level(&spi_ss_pin, true);
	
}	


	
	
void spi_write_byte(uint8_t data){
	spi_txrx_done = 0;
	SPDR = data;
	while(spi_txrx_done == 0);
}	

void soft_spi_write_byte(uint8_t data){
	for(uint8_t n=8; n!=0; n--)
	{	
		if((1 << (n-1)) & data){
			gpio_set_pin_level(&spi_tx_pin, true);
			}else{
			gpio_set_pin_level(&spi_tx_pin, false);
		}	
		gpio_set_pin_level(&spi_clk_pin, true);
		gpio_set_pin_level(&spi_clk_pin, false);	
	}
}

uint8_t spi_read_byte(void){
	spi_write_byte(0);
return SPDR;
}

uint8_t soft_spi_read_byte(void){
	uint8_t	result;
	for(uint8_t n=8; n!=0; n--){
		gpio_set_pin_level(&spi_clk_pin, true);
		gpio_set_pin_level(&spi_clk_pin, true);
		result <<=1;
		result += gpio_get_pin_level(&spi_rx_pin);
		gpio_set_pin_level(&spi_clk_pin, false);
	}
	return result;
}

void spi_tranfer(uint8_t *tx,uint16_t len){
	for(uint16_t i=0; i < len;i++){
		spi_send(&tx[i]);
	}
}

void spi_send(uint8_t *tx){
	spi_txrx_done = 0;
	SPDR = *tx;
	while(spi_txrx_done == 0);
	
}