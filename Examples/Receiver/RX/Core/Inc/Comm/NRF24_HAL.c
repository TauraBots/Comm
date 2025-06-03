#include <Comm/NRF24_HAL.h>

void NRF24_HAL_CE_Enable(void) {
    HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_SET);
}

void NRF24_HAL_CE_Disable(void) {
    HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET);
}

void NRF24_HAL_CS_Select(void) {
    HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_RESET);
}

void NRF24_HAL_CS_UnSelect(void) {
    HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET);
}

HAL_StatusTypeDef NRF24_HAL_SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    return HAL_SPI_Transmit(NRF24_SPI, pData, Size, Timeout);
}

HAL_StatusTypeDef NRF24_HAL_SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout) {
    return HAL_SPI_Receive(NRF24_SPI, pData, Size, Timeout);
}

void NRF24_HAL_Delay(uint32_t milliseconds) {
    HAL_Delay(milliseconds);
}

uint32_t NRF24_HAL_GetTick(void) {
    return HAL_GetTick();
}
