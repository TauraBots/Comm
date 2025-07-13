// Comm/NRF24_CORE.hpp
#ifndef NRF24_CORE_HPP_ // Alterada guarda de inclusão
#define NRF24_CORE_HPP_ // Alterada guarda de inclusão

#include <Comm/NRF24_DEF.hpp> // Atualizado caminho de inclusão
#include <stdint.h>


#ifdef __cplusplus // Adicionado bloco extern "C"
extern "C" {
#endif

void nrf24_WriteReg(uint8_t Reg, uint8_t Data);
void nrf24_WriteRegMulti(uint8_t Reg, uint8_t *data, uint8_t size);
uint8_t nrf24_ReadReg(uint8_t Reg);
void nrf24_ReadReg_Multi(uint8_t Reg, uint8_t *data, uint8_t size);
void nrfsendCmd(uint8_t cmd);


void nrf24_reset_registers(void);
void nrf24_clear_interrupts(void);
void nrf24_flush_tx(void);
void nrf24_flush_rx(void);

void NRF24_Init(void);

void NRF24_TxMode(uint8_t *Address, uint8_t channel);
uint8_t NRF24_Transmit(uint8_t *data, uint8_t size);


void NRF24_RxMode(uint8_t *AddressPipe1, uint8_t AddressPipe2LSB, uint8_t channel);

uint8_t isDataAvailable(uint8_t pipenum);
void NRF24_Receive(uint8_t *data);

void NRF24_ReadAll(uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif