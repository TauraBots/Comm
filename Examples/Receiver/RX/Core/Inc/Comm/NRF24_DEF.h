#ifndef NRF24_DEF_H_
#define NRF24_DEF_H_

#include "stm32f4xx_hal.h" 

#define NRF24_CE_PORT   GPIOB
#define NRF24_CE_PIN    GPIO_PIN_1

#define NRF24_CSN_PORT  GPIOB
#define NRF24_CSN_PIN   GPIO_PIN_0


extern SPI_HandleTypeDef hspi1;
#define NRF24_SPI       &hspi1


#define NRF24_MAX_RETRANSMISSIONS 15


#define CONFIG        0x00
#define EN_AA         0x01
#define EN_RXADDR     0x02
#define SETUP_AW      0x03
#define SETUP_RETR    0x04
#define RF_CH         0x05
#define RF_SETUP      0x06
#define STATUS        0x07
#define OBSERVE_TX    0x08
#define CD            0x09 // Carrier Detect (ou RPD - Received Power Detector)
#define RX_ADDR_P0    0x0A
#define RX_ADDR_P1    0x0B
#define RX_ADDR_P2    0x0C
#define RX_ADDR_P3    0x0D
#define RX_ADDR_P4    0x0E
#define RX_ADDR_P5    0x0F
#define TX_ADDR       0x10
#define RX_PW_P0      0x11
#define RX_PW_P1      0x12
#define RX_PW_P2      0x13
#define RX_PW_P3      0x14
#define RX_PW_P4      0x15
#define RX_PW_P5      0x16
#define FIFO_STATUS   0x17
#define DYNPD         0x1C
#define FEATURE       0x1D


#define RX_DR_BIT     6
#define TX_DS_BIT     5
#define MAX_RT_BIT    4
#define RX_P_NO_MASK  0x0E
#define RX_P_NO_POS   1
#define TX_FULL_BIT   0


#define R_REGISTER    0x00
#define W_REGISTER    0x20
#define REGISTER_MASK 0x1F

#define ACTIVATE      0x50
#define R_RX_PL_WID   0x60
#define R_RX_PAYLOAD  0x61
#define W_TX_PAYLOAD  0xA0
#define W_ACK_PAYLOAD 0xA8

#define FLUSH_TX      0xE1
#define FLUSH_RX      0xE2
#define REUSE_TX_PL   0xE3
#define NOP           0xFF

#endif
