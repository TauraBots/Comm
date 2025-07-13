/**
 * @file NRF24_DEF.hpp
 * @brief Definições de hardware e configurações específicas para os módulos NRF24L01+.
 *
 * Este arquivo foi refatorado para suportar múltiplos módulos NRF24L01+ no mesmo microcontrolador STM32,
 * conforme a nova arquitetura de comunicação para robôs (um NRF para RX, um para TX) e torre de comando
 * (um NRF para RX de telemetria, cinco para TX de comandos).
 *
 * A lógica original de definição de registradores NRF24 e comandos foi mantida.
 * A principal alteração é a introdução de uma estrutura `NRF24_Hardware_Config_t`
 * que encapsula as configurações de hardware (SPI_HandleTypeDef, portas e pinos GPIO para CE e CSN)
 * para cada módulo NRF24 individualmente. Isso substitui as macros globais anteriores
 * que assumiam um único NRF.
 *
 * As configurações de pinos e instâncias SPI são agora definidas de forma mais organizada,
 * permitindo a fácil alocação de recursos para cada NRF. As definições aqui são exemplos
 * e devem ser ajustadas para refletir a pinagem e os periféricos SPI utilizados em seu hardware específico.
 *
 * Contexto e Metodologia:
 * - **Manutenção da Lógica Original**: Os valores e nomes dos registradores NRF24 e comandos
 * foram preservados para garantir compatibilidade e aderência à folha de dados do módulo.
 * - **Modularidade e Escalabilidade**: A nova estrutura facilita o gerenciamento de múltiplos
 * módulos NRF24, tornando o código mais legível e escalável para cenários mais complexos.
 * - **Pragmatismo**: Fornece um template claro para a definição de hardware, exigindo apenas
 * ajustes nos valores específicos para o seu setup, sem alterar a arquitetura da biblioteca.
 * - **Abstração**: Move a complexidade das definições de hardware para um local centralizado,
 * permitindo que as camadas superiores (`NRF24_CORE` e `COMM`) operem com instâncias abstratas do NRF.
 */

#ifndef NRF24_DEF_HPP
#define NRF24_DEF_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h" // Inclui o HAL da STM32 para tipos como SPI_HandleTypeDef e GPIO_TypeDef

// --- Definições de Registradores NRF24L01+ (Mantidas do original) ---
// Comandos
#define R_REGISTER    0x00U
#define W_REGISTER    0x20U
#define R_RX_PAYLOAD  0x61U
#define W_TX_PAYLOAD  0xA0U
#define FLUSH_TX      0xE1U
#define FLUSH_RX      0xE2U
#define REUSE_TX_PL   0xE3U
#define ACTIVATE      0x50U
#define R_RX_PL_WID   0x60U
#define NOP           0xFFU

// Registradores
#define CONFIG        0x00U
#define EN_AA         0x01U
#define EN_RXADDR     0x02U
#define SETUP_AW      0x03U
#define SETUP_RETR    0x04U
#define RF_CH         0x05U
#define RF_SETUP      0x06U
#define STATUS        0x07U
#define OBSERVE_TX    0x08U
#define CD            0x09U
#define RX_ADDR_P0    0x0AU
#define RX_ADDR_P1    0x0BU
#define RX_ADDR_P2    0x0CU
#define RX_ADDR_P3    0x0DU
#define RX_ADDR_P4    0x0EU
#define RX_ADDR_P5    0x0FU
#define TX_ADDR       0x10U
#define RX_PW_P0      0x11U
#define RX_PW_P1      0x12U
#define RX_PW_P2      0x13U
#define RX_PW_P3      0x14U
#define RX_PW_P4      0x15U
#define RX_PW_P5      0x16U
#define FIFO_STATUS   0x17U
#define DYNPD         0x1CU
#define FEATURE       0x1DU

// Bits dos Registradores
#define MASK_RX_DR    6
#define MASK_TX_DS    5
#define MASK_MAX_RT   4
#define EN_CRC        3
#define CRCO          2
#define PWR_UP        1
#define PRIM_RX       0
#define ENAA_P5       5
#define ENAA_P4       4
#define ENAA_P3       3
#define ENAA_P2       2
#define ENAA_P1       1
#define ENAA_P0       0
#define ERX_P5        5
#define ERX_P4        4
#define ERX_P3        3
#define ERX_P2        2
#define ERX_P1        1
#define ERX_P0        0
#define AW            0
#define ARD           4
#define ARC           0
#define PLL_LOCK      4
#define RF_DR_LOW     5
#define RF_DR_HIGH    3
#define RF_PWR        1
#define RX_DR         6
#define TX_DS         5
#define MAX_RT        4
#define RX_P_NO       1
#define TX_FULL       0
#define APL           2
#define RPD           0
#define TX_REUSE      6
#define TX_FIFO_FULL  5
#define TX_EMPTY      4
#define RX_FIFO_FULL  1
#define RX_FIFO_EMPTY 0
#define DPL_P5        5
#define DPL_P4        4
#define DPL_P3        3
#define DPL_P2        2
#define DPL_P1        1
#define EN_DPL        2
#define EN_ACK_PAY    1
#define EN_DYN_ACK    0

// --- Definições de Hardware para Múltiplos NRF24L01+ ---

/**
 * @brief Estrutura para configurar o hardware de um módulo NRF24L01+ individual.
 * @note Esta estrutura substitui as macros globais de pinagem e handle SPI.
 * Permite que cada instância de NRF24 tenha sua própria configuração de pinos CE/CSN
 * e um ponteiro para a instância SPI do HAL.
 */
typedef struct {
    SPI_HandleTypeDef* hspi;    ///< Ponteiro para o handle SPI do HAL (ex: &hspi1)
    GPIO_TypeDef* ce_port;      ///< Porta GPIO para o pino CE (Chip Enable)
    uint16_t ce_pin;            ///< Número do pino CE
    GPIO_TypeDef* csn_port;     ///< Porta GPIO para o pino CSN (Chip Select Not)
    uint16_t csn_pin;           ///< Número do pino CSN
    uint32_t spi_timeout_ms;    ///< Tempo limite em ms para operações SPI
} NRF24_Hardware_Config_t;

// --- Configurações de Hardware de Exemplo para Robô e Torre ---
// As definições abaixo são exemplos e DEVEM SER ADAPTADAS à pinagem real do seu hardware.
// Você pode ter diferentes handles SPI (ex: hspi1, hspi2) ou usar o mesmo com CSNs distintos.

// Robô (Receiver): 1 NRF para RX, 1 NRF para TX
// NRF de Recepção de Comandos (Robô)
// Alteração: Define a configuração de hardware para o NRF de RX do robô.
// Este NRF receberá comandos da torre.
#define ROBOT_RX_NRF_HW_CONFIG { &hspi1, GPIOB, GPIO_PIN_1, GPIOA, GPIO_PIN_4, 100 } // Exemplo: SPI1, CE=PB1, CSN=PA4

// NRF de Transmissão de Telemetria (Robô)
// Alteração: Define a configuração de hardware para o NRF de TX do robô.
// Este NRF enviará dados de telemetria para a torre.
#define ROBOT_TX_NRF_HW_CONFIG { &hspi2, GPIOC, GPIO_PIN_2, GPIOD, GPIO_PIN_3, 100 } // Exemplo: SPI2, CE=PC2, CSN=PD3

// Torre (Transmitter): 1 NRF para RX de telemetria, 5 NRFs para TX de comandos
// NRF de Recepção de Telemetria (Torre)
// Alteração: Define a configuração de hardware para o NRF de RX da torre.
// Este NRF receberá telemetria de todos os robôs.
#define TOWER_TELEMETRY_RX_NRF_HW_CONFIG { &hspi1, GPIOA, GPIO_PIN_1, GPIOB, GPIO_PIN_0, 100 } // Exemplo: SPI1, CE=PA1, CSN=PB0

// NRFs de Transmissão de Comandos (Torre) - Um para cada robô (até 5)
// Alteração: Define um array de configurações de hardware para os 5 NRFs de TX da torre.
// Cada NRF neste array será responsável por enviar comandos para um robô específico.
// Os handles SPI e pinos CE/CSN devem ser únicos para cada NRF físico.
#define TOWER_COMMAND_TX_NRF_HW_CONFIGS { \
    { &hspi2, GPIOC, GPIO_PIN_1, GPIOD, GPIO_PIN_1, 100 },  /* Robo 1 */ \
    { &hspi3, GPIOE, GPIO_PIN_2, GPIOF, GPIO_PIN_2, 100 },  /* Robo 2 */ \
    { &hspi4, GPIOA, GPIO_PIN_5, GPIOB, GPIO_PIN_6, 100 },  /* Robo 3 */ \
    { &hspi5, GPIOC, GPIO_PIN_7, GPIOD, GPIO_PIN_8, 100 },  /* Robo 4 */ \
    { &hspi6, GPIOE, GPIO_PIN_9, GPIOF, GPIO_PIN_10, 100 }   /* Robo 5 */ \
}

// Endereços de Comunicação (Mantidos do original, com pequenas adaptações para clareza)
// Alteração: Os endereços agora são tipicamente usados para configurar as instâncias NRF24_Driver.
// ADDR_BASE_LISTEN: Endereço base para escuta da torre para telemetria dos robôs.
//   Os robôs transmitirão para este endereço quando enviarem telemetria.
#define ADDR_BASE_LISTEN {0xAA, 0xAA, 0xAA, 0xAA, 0xAA}

// ADDR_ROBOT_LISTEN: Endereços de escuta para cada robô.
//   A torre transmitirá para o endereço específico de cada robô.
//   O robô receberá neste endereço.
#define ADDR_ROBOT_LISTEN_0 {0xBB, 0xBB, 0xBB, 0xBB, 0xBB} // Robô 0 (Exemplo, se houver um robô 0)
#define ADDR_ROBOT_LISTEN_1 {0xC1, 0xC1, 0xC1, 0xC1, 0xC1} // Robô 1
#define ADDR_ROBOT_LISTEN_2 {0xC2, 0xC2, 0xC2, 0xC2, 0xC2} // Robô 2
#define ADDR_ROBOT_LISTEN_3 {0xC3, 0xC3, 0xC3, 0xC3, 0xC3} // Robô 3
#define ADDR_ROBOT_LISTEN_4 {0xC4, 0xC4, 0xC4, 0xC4, 0xC4} // Robô 4
#define ADDR_ROBOT_LISTEN_5 {0xC5, 0xC5, 0xC5, 0xC5, 0xC5} // Robô 5

// Array de conveniência para acessar os endereços dos robôs por ID
// Alteração: Cria um array para acessar facilmente os endereços de escuta dos robôs.
// Útil para a torre ao enviar comandos para um robô específico.
static const uint8_t ROBOT_LISTEN_ADDRESSES[][5] = {
    ADDR_ROBOT_LISTEN_0,
    ADDR_ROBOT_LISTEN_1,
    ADDR_ROBOT_LISTEN_2,
    ADDR_ROBOT_LISTEN_3,
    ADDR_ROBOT_LISTEN_4,
    ADDR_ROBOT_LISTEN_5
};

// Tamanho do Payload (Mantido do original)
#define MAX_PAYLOAD_SIZE 32U

#ifdef __cplusplus
}
#endif

#endif // NRF24_DEF_HPP