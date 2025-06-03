#ifndef NRF24_HAL_H_
#define NRF24_HAL_H_

#include <Comm/NRF24_DEF.h>

void NRF24_HAL_CE_Enable(void);
void NRF24_HAL_CE_Disable(void);
void NRF24_HAL_CS_Select(void);
void NRF24_HAL_CS_UnSelect(void);

HAL_StatusTypeDef NRF24_HAL_SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef NRF24_HAL_SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout);

void NRF24_HAL_Delay(uint32_t milliseconds);
uint32_t NRF24_HAL_GetTick(void);

#endif
