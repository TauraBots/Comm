#include "NRF24_HAL.hpp"
#include "NRF24_DEF.hpp" // Para incluir definições como NRF24_HAL_Delay, se ainda forem usadas globalmente ou para o construtor.

// O arquivo NRF24_HAL.cpp agora implementa os métodos da classe NRF24_Hardware.

// Construtor da classe NRF24_Hardware
// Este construtor inicializa a instância de hardware com os parâmetros específicos do NRF.
// Cada NRF terá seu próprio conjunto de pinos CE/CSN e, potencialmente, seu próprio handle SPI.
NRF24_Hardware::NRF24_Hardware(SPI_HandleTypeDef* hspi_handle, 
                               GPIO_TypeDef* ce_gpio_port, uint16_t ce_gpio_pin,
                               GPIO_TypeDef* csn_gpio_port, uint16_t csn_gpio_pin,
                               uint32_t spi_timeout)
    : hspi(hspi_handle),
      ce_port(ce_gpio_port),
      ce_pin(ce_gpio_pin),
      csn_port(csn_gpio_port),
      csn_pin(csn_gpio_pin),
      spi_timeout_ms(spi_timeout)
{
    // O construtor apenas armazena os parâmetros.
    // A inicialização real do NRF (registros, modos) será feita na camada NRF24_Driver.
}

// Habilita o pino CE (Chip Enable) do NRF24, usado para ativar os modos TX ou RX.
void NRF24_Hardware::CE_Enable() {
    HAL_GPIO_WritePin(ce_port, ce_pin, GPIO_PIN_SET);
}

// Desabilita o pino CE (Chip Enable) do NRF24.
void NRF24_Hardware::CE_Disable() {
    HAL_GPIO_WritePin(ce_port, ce_pin, GPIO_PIN_RESET);
}

// Seleciona o NRF24 para comunicação SPI, ativando o pino CSN (Chip Select Not).
void NRF24_Hardware::CS_Select() {
    HAL_GPIO_WritePin(csn_port, csn_pin, GPIO_PIN_RESET);
}

// Desseleciona o NRF24 da comunicação SPI, desativando o pino CSN.
void NRF24_Hardware::CS_UnSelect() {
    HAL_GPIO_WritePin(csn_port, csn_pin, GPIO_PIN_SET);
}

// Transmite dados via SPI para o NRF24.
HAL_StatusTypeDef NRF24_Hardware::SPI_Transmit(uint8_t* pData, uint16_t Size) {
    // Agora usa o handle SPI e o timeout específicos desta instância de hardware.
    return HAL_SPI_Transmit(hspi, pData, Size, spi_timeout_ms);
}

// Recebe dados via SPI do NRF24.
HAL_StatusTypeDef NRF24_Hardware::SPI_Receive(uint8_t* pData, uint16_t Size) {
    // Agora usa o handle SPI e o timeout específicos desta instância de hardware.
    return HAL_SPI_Receive(hspi, pData, Size, spi_timeout_ms);
}

// Transmite e recebe dados simultaneamente via SPI (full-duplex) do NRF24.
HAL_StatusTypeDef NRF24_Hardware::SPI_TransmitReceive(uint8_t* pTxData, uint8_t* pRxData, uint16_t Size) {
    // Agora usa o handle SPI e o timeout específicos desta instância de hardware.
    return HAL_SPI_TransmitReceive(hspi, pTxData, pRxData, Size, spi_timeout_ms);
}

// Função de atraso em milissegundos.
// Esta função pode ser uma função global ou um método da classe se o tick for gerenciado pela instância.
// Por simplicidade, mantida como função global aqui ou pode ser um método estático.
void NRF24_Hardware::Delay(uint32_t Delay_ms) {
    HAL_Delay(Delay_ms);
}

// Retorna o valor do tick do sistema, usado para temporização.
// Esta função pode ser uma função global ou um método da classe se o tick for gerenciado pela instância.
// Por simplicidade, mantida como função global aqui ou pode ser um método estático.
uint32_t NRF24_Hardware::GetTick() {
    return HAL_GetTick();
}