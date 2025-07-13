#ifndef NRF24_CORE_HPP_
#define NRF24_CORE_HPP_

#include "main.h"        // Inclui HAL_Delay, HAL_GPIO_WritePin, HAL_SPI_Transmit
#include "NRF24_DEF.hpp" // Inclui definições de registradores e pinos
#include "NRF24_HAL.hpp" // Inclui a nova classe NRF24_Hardware

// Definições de tamanhos de endereço (mantidas para consistência)
#define NRF_ADDR_SIZE       5
#define NRF_RX_PIPE_ADDR_SIZE_P0_P1 5
#define NRF_RX_PIPE_ADDR_SIZE_P2_P3_P4_P5 1

// Enums para configurações (mantidas como globais para consistência com o original)
typedef enum {
    NRF_DATARATE_250KBPS = 0b10,
    NRF_DATARATE_1MBPS = 0b00,
    NRF_DATARATE_2MBPS = 0b01
} nrf_datarate_t;

typedef enum {
    NRF_TX_POWER_M18DBM = 0b00,
    NRF_TX_POWER_M12DBM = 0b01,
    NRF_TX_POWER_M6DBM = 0b10,
    NRF_TX_POWER_0DBM = 0b11
} nrf_tx_power_t;

typedef enum {
    NRF_CRC_DISABLED = 0b00,
    NRF_CRC_1_BYTE = 0b01,
    NRF_CRC_2_BYTES = 0b10
} nrf_crc_t;

class NRF24_Driver {
public:
    /**
     * @brief Construtor da classe NRF24_Driver.
     *
     * @param hspi Ponteiro para o handle SPI do HAL (ex: &hspi1).
     * @param ce_port Porta GPIO para o pino CE.
     * @param ce_pin Pino GPIO para o CE.
     * @param csn_port Porta GPIO para o pino CSN.
     * @param csn_pin Pino GPIO para o CSN.
     * @param spi_timeout_ms Tempo limite em milissegundos para operações SPI.
     *
     * @change: Novo construtor para inicializar a instância NRF24_Hardware para cada NRF.
     * Isso permite que cada objeto NRF24_Driver controle um NRF físico específico.
     */
    NRF24_Driver(SPI_HandleTypeDef* hspi, GPIO_TypeDef* ce_port, uint16_t ce_pin, GPIO_TypeDef* csn_port, uint16_t csn_pin, uint32_t spi_timeout_ms);

    /**
     * @brief Inicializa o módulo NRF24 com as configurações fornecidas.
     *
     * @param rx_pipe0_address Endereço de recepção para o Pipe 0 (5 bytes).
     * @param rx_pipe1_address Endereço de recepção para o Pipe 1 (5 bytes).
     * @param tx_address Endereço de transmissão (5 bytes).
     * @param channel Canal RF (0-125).
     * @param datarate Taxa de dados (NRF_DATARATE_250KBPS, NRF_DATARATE_1MBPS, NRF_DATARATE_2MBPS).
     * @param tx_power Potência de transmissão (NRF_TX_POWER_M18DBM a NRF_TX_POWER_0DBM).
     * @param auto_ack_enabled Habilita/desabilita Auto Acknowledgment.
     * @param retransmit_count Número de retransmissões automáticas (0-15).
     * @param retransmit_delay_us Atraso entre retransmissões em microssegundos (250-4000).
     * @param crc_mode Modo CRC (NRF_CRC_DISABLED, NRF_CRC_1_BYTE, NRF_CRC_2_BYTES).
     * @param dynamic_payload_enabled Habilita/desabilita payloads dinâmicos.
     * @return true se a inicialização for bem-sucedida, false caso contrário.
     *
     * @change: Método Init agora é um membro da classe e recebe todos os parâmetros de configuração.
     * Isso permite configurar individualmente cada NRF em um sistema multi-NRF.
     */
    bool Init(uint8_t rx_pipe0_address[NRF_ADDR_SIZE], uint8_t rx_pipe1_address[NRF_ADDR_SIZE],
              uint8_t tx_address[NRF_ADDR_SIZE], uint8_t channel,
              nrf_datarate_t datarate, nrf_tx_power_t tx_power,
              bool auto_ack_enabled, uint8_t retransmit_count, uint16_t retransmit_delay_us,
              nrf_crc_t crc_mode, bool dynamic_payload_enabled);

    /**
     * @brief Coloca o NRF24 no modo de Recepção (RX).
     * @change: Convertido de função global para método da classe.
     */
    void RxMode();

    /**
     * @brief Coloca o NRF24 no modo de Transmissão (TX).
     * @change: Convertido de função global para método da classe.
     */
    void TxMode();

    /**
     * @brief Transmite um pacote de dados.
     *
     * @param data Ponteiro para os dados a serem transmitidos.
     * @param size Tamanho dos dados em bytes.
     * @param block_until_sent Se true, a função bloqueará até o pacote ser enviado ou falhar.
     * @return true se o pacote foi enviado com sucesso, false caso contrário.
     * @change: Convertido de função global para método da classe.
     */
    bool Transmit(uint8_t* data, uint8_t size, bool block_until_sent = true);

    /**
     * @brief Verifica se um pacote está disponível para leitura no RX FIFO.
     *
     * @param pipe_num Ponteiro opcional para retornar o número do pipe onde o pacote foi recebido.
     * @return true se um pacote estiver disponível.
     * @change: Convertido de função global para método da classe.
     */
    bool IsPacketAvailable(uint8_t* pipe_num = nullptr);

    /**
     * @brief Lê um pacote do RX FIFO.
     *
     * @param data Buffer para armazenar os dados lidos.
     * @param size Tamanho máximo do buffer de dados.
     * @return true se o pacote foi lido com sucesso.
     * @change: Convertido de função global para método da classe.
     */
    bool ReadPacket(uint8_t* data, uint8_t size);

    /**
     * @brief Escreve um valor em um registrador do NRF24.
     *
     * @param reg Endereço do registrador.
     * @param data Valor a ser escrito.
     * @change: Convertido de função global para método da classe.
     */
    void WriteReg(uint8_t reg, uint8_t data);

    /**
     * @brief Lê o valor de um registrador do NRF24.
     *
     * @param reg Endereço do registrador.
     * @return O valor lido.
     * @change: Convertido de função global para método da classe.
     */
    uint8_t ReadReg(uint8_t reg);

    /**
     * @brief Envia um comando para o NRF24 e retorna o byte de status.
     *
     * @param cmd O comando a ser enviado.
     * @return O byte de status lido após o comando.
     * @change: Convertido de função global para método da classe.
     */
    uint8_t SendCommand(uint8_t cmd);

    /**
     * @brief Envia um comando, lê dados subsequentes e retorna o byte de status.
     *
     * @param cmd O comando a ser enviado.
     * @param data Buffer para armazenar os dados lidos.
     * @param size Tamanho dos dados a serem lidos.
     * @return O byte de status lido após a operação.
     * @change: Convertido de função global para método da classe.
     */
    uint8_t SendCommandAndRead(uint8_t cmd, uint8_t* data, uint8_t size);

    /**
     * @brief Envia um comando e escreve dados subsequentes.
     *
     * @param cmd O comando a ser enviado.
     * @param data Dados a serem escritos.
     * @param size Tamanho dos dados a serem escritos.
     * @change: Convertido de função global para método da classe.
     */
    void SendCommandAndWrite(uint8_t cmd, uint8_t* data, uint8_t size);

    /**
     * @brief Configura o endereço de um pipe de recepção específico.
     *
     * @param pipe_num Número do pipe (0 a 5).
     * @param address Ponteiro para o array de bytes do endereço.
     * @change: Novo método para configurar endereços de pipes RX individualmente.
     */
    void SetRxAddress(uint8_t pipe_num, uint8_t* address);

    /**
     * @brief Configura o endereço de transmissão.
     *
     * @param address Ponteiro para o array de bytes do endereço.
     * @change: Novo método para configurar o endereço TX individualmente.
     */
    void SetTxAddress(uint8_t* address);

    /**
     * @brief Configura a potência de transmissão.
     * @param power A potência de transmissão desejada.
     * @change: Novo método para configurar a potência de TX.
     */
    void SetTxPower(nrf_tx_power_t power);

    /**
     * @brief Configura a taxa de dados.
     * @param rate A taxa de dados desejada.
     * @change: Novo método para configurar a taxa de dados.
     */
    void SetDataRate(nrf_datarate_t rate);

    /**
     * @brief Configura o canal RF.
     * @param ch O canal RF desejado (0-125).
     * @change: Novo método para configurar o canal RF.
     */
    void SetChannel(uint8_t ch);

    /**
     * @brief Habilita o Auto Acknowledgment (Auto ACK).
     * @change: Novo método para habilitar Auto ACK.
     */
    void EnableAutoAck();

    /**
     * @brief Desabilita o Auto Acknowledgment (Auto ACK).
     * @change: Novo método para desabilitar Auto ACK.
     */
    void DisableAutoAck();

    /**
     * @brief Habilita payloads dinâmicos.
     * @change: Novo método para habilitar payloads dinâmicos.
     */
    void EnableDynamicPayloads();

    /**
     * @brief Desabilita payloads dinâmicos.
     * @change: Novo método para desabilitar payloads dinâmicos.
     */
    void DisableDynamicPayloads();

    /**
     * @brief Configura o número de retransmissões e o atraso entre elas.
     * @param count Número de retransmissões.
     * @param delay_us Atraso em microssegundos.
     * @change: Novo método para configurar retransmissões.
     */
    void SetRetransmission(uint8_t count, uint16_t delay_us);

    /**
     * @brief Obtém o byte de status atual do NRF24.
     * @return O byte de status.
     * @change: Convertido de função global para método da classe.
     */
    uint8_t GetStatus();

    /**
     * @brief Limpa as flags de interrupção (RX_DR, TX_DS, MAX_RT).
     * @change: Convertido de função global para método da classe.
     */
    void ClearInterrupts();

    /**
     * @brief Esvazia o buffer de transmissão (TX FIFO).
     * @change: Convertido de função global para método da classe.
     */
    void FlushTxFIFO();

    /**
     * @brief Esvazia o buffer de recepção (RX FIFO).
     * @change: Convertido de função global para método da classe.
     */
    void FlushRxFIFO();

    /**
     * @brief Obtém o status do FIFO (vazio, cheio, etc.).
     * @return O byte de status do FIFO.
     * @change: Convertido de função global para método da classe.
     */
    uint8_t GetFIFOStatus();

    /**
     * @brief Obtém o tamanho do payload dinâmico para o pacote principal (TOP FIFO).
     * @return O tamanho do payload dinâmico.
     * @change: Convertido de função global para método da classe.
     */
    uint8_t GetDynamicPayloadSize();

private:
    NRF24_Hardware hardware; // @change: Instância da camada de hardware para este NRF.

    // @change: Membros para armazenar a configuração atual desta instância NRF24.
    // Isso permite que cada NRF_Driver mantenha seu próprio estado de configuração.
    uint8_t current_tx_address[NRF_ADDR_SIZE];
    uint8_t current_rx_addresses[6][NRF_ADDR_SIZE];
    uint8_t current_channel;
    nrf_datarate_t current_datarate;
    nrf_tx_power_t current_tx_power;
    bool current_auto_ack_enabled;
    uint8_t current_retransmit_count;
    uint16_t current_retransmit_delay_us;
    nrf_crc_t current_crc_mode;
    bool current_dynamic_payload_enabled;

    /**
     * @brief Método auxiliar privado para configurar o endereço de um pipe de recepção.
     * @param pipe_num Número do pipe (0 a 5).
     * @param address Ponteiro para o array de bytes do endereço.
     * @param size Tamanho do endereço.
     * @change: Método auxiliar privado para gerenciar a lógica interna de configuração de endereços de pipe.
     */
    void set_rx_pipe_address_internal(uint8_t pipe_num, uint8_t* address, uint8_t size);
};

#endif /* NRF24_CORE_HPP_ */