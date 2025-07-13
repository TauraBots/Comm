#ifndef __NRF24_HAL_HPP
#define __NRF24_HAL_HPP

#include "main.h" // Inclui o cabeçalho principal do STM32 HAL para acesso às funções e tipos de dados

// Removemos as definições de pinos e handle SPI globais (NRF24_CE_PORT, NRF24_SPI_HANDLE, etc.)
// Estas configurações agora serão encapsuladas dentro da classe NRF24_Hardware,
// permitindo múltiplas instâncias do NRF24 com diferentes configurações de hardware.

// Classe NRF24_Hardware
// Encapsula as operações de hardware de baixo nível para um único módulo NRF24L01+.
// Isso permite que o NRF24_CORE (e, por sua vez, a camada COMM) gerencie múltiplas
// instâncias de NRFs, cada uma com sua própria configuração de pinos e SPI.
class NRF24_Hardware {
private:
    SPI_HandleTypeDef* hspi;     // Ponteiro para o handle SPI do STM32 HAL
    GPIO_TypeDef* ce_port;       // Porta GPIO para o pino CE (Chip Enable)
    uint16_t ce_pin;             // Número do pino CE
    GPIO_TypeDef* csn_port;      // Porta GPIO para o pino CSN (Chip Select Not)
    uint16_t csn_pin;            // Número do pino CSN
    uint32_t spi_timeout_ms;     // Tempo limite para operações SPI em milissegundos

public:
    // Construtor da classe NRF24_Hardware
    // Inicializa a instância de hardware com as configurações específicas do NRF.
    // @param spi_handle: Ponteiro para o handle SPI (ex: &hspi1).
    // @param ce_gpio_port: Ponteiro para a porta GPIO do pino CE (ex: GPIOB).
    // @param ce_gpio_pin: Número do pino CE (ex: GPIO_PIN_1).
    // @param csn_gpio_port: Ponteiro para a porta GPIO do pino CSN (ex: GPIOA).
    // @param csn_gpio_pin: Número do pino CSN (ex: GPIO_PIN_4).
    // @param timeout_ms: Tempo limite para operações SPI em milissegundos.
    NRF24_Hardware(SPI_HandleTypeDef* spi_handle, GPIO_TypeDef* ce_gpio_port, uint16_t ce_gpio_pin,
                   GPIO_TypeDef* csn_gpio_port, uint16_t csn_gpio_pin, uint32_t timeout_ms)
        : hspi(spi_handle), ce_port(ce_gpio_port), ce_pin(ce_gpio_pin),
          csn_port(csn_gpio_port), csn_pin(csn_gpio_pin), spi_timeout_ms(timeout_ms) {
        // O corpo do construtor pode estar vazio se a inicialização for feita na lista de inicialização de membros.
    }

    // Métodos para controlar o pino CE (Chip Enable)
    // Anteriormente: NRF24_HAL_CE_Enable() e NRF24_HAL_CE_Disable()
    // Alteração: Tornados métodos da classe, utilizando os membros privados da instância.
    void CE_Enable() {
        HAL_GPIO_WritePin(ce_port, ce_pin, GPIO_PIN_SET); // Seta o pino CE (habilita o chip NRF)
    }

    void CE_Disable() {
        HAL_GPIO_WritePin(ce_port, ce_pin, GPIO_PIN_RESET); // Reseta o pino CE (desabilita o chip NRF)
    }

    // Métodos para controlar o pino CSN (Chip Select Not)
    // Anteriormente: NRF24_HAL_CS_Select() e NRF24_HAL_CS_UnSelect()
    // Alteração: Tornados métodos da classe, utilizando os membros privados da instância.
    void CS_Select() {
        HAL_GPIO_WritePin(csn_port, csn_pin, GPIO_PIN_RESET); // Reseta o pino CSN (seleciona o chip NRF)
    }

    void CS_UnSelect() {
        HAL_GPIO_WritePin(csn_port, csn_pin, GPIO_PIN_SET); // Seta o pino CSN (desseleciona o chip NRF)
    }

    // Métodos para comunicação SPI
    // Anteriormente: NRF24_HAL_SPI_Transmit(), NRF24_HAL_SPI_Receive(), NRF24_HAL_SPI_TransmitReceive()
    // Alteração: Tornados métodos da classe, utilizando o handle SPI e o timeout da instância.
    // As funções HAL_SPI_... são bloqueantes com timeout.

    // Transmite dados via SPI
    // @param pData: Ponteiro para o buffer de dados a ser transmitido.
    // @param Size: Número de bytes a serem transmitidos.
    HAL_StatusTypeDef SPI_Transmit(uint8_t* pData, uint16_t Size) {
        return HAL_SPI_Transmit(hspi, pData, Size, spi_timeout_ms);
    }

    // Recebe dados via SPI
    // @param pData: Ponteiro para o buffer onde os dados recebidos serão armazenados.
    // @param Size: Número de bytes a serem recebidos.
    HAL_StatusTypeDef SPI_Receive(uint8_t* pData, uint16_t Size) {
        return HAL_SPI_Receive(hspi, pData, Size, spi_timeout_ms);
    }

    // Transmite e recebe dados simultaneamente via SPI
    // @param pTxData: Ponteiro para o buffer de dados a ser transmitido.
    // @param pRxData: Ponteiro para o buffer onde os dados recebidos serão armazenados.
    // @param Size: Número de bytes a serem transmitidos/recebidos.
    HAL_StatusTypeDef SPI_TransmitReceive(uint8_t* pTxData, uint8_t* pRxData, uint16_t Size) {
        return HAL_SPI_TransmitReceive(hspi, pTxData, pRxData, Size, spi_timeout_ms);
    }

    // Métodos para atraso e obtenção de tick
    // Anteriormente: NRF24_HAL_Delay(), NRF24_HAL_GetTick()
    // Alteração: Mantidos como métodos estáticos ou podem ser chamados diretamente se não precisarem de membros da classe.
    // Para manter a consistência e a possibilidade de futura injeção de dependência de tempo,
    // podemos mantê-los como métodos, mesmo que sejam apenas wrappers para funções HAL.
    static void Delay(uint32_t Delay) {
        HAL_Delay(Delay);
    }

    static uint32_t GetTick() {
        return HAL_GetTick();
    }
};

#endif // __NRF24_HAL_HPP