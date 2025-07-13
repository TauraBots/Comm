#ifndef COMM_HPP_ // Guarda de inclusão (mantida)
#define COMM_HPP_ // Guarda de inclusão (mantida)

#include <Comm/COMM_PACKETS.hpp> // Atualizado caminho de inclusão (mantido)
#include <Comm/NRF24_CORE.hpp> // **Alteração:** Agora este cabeçalho deve definir a nova classe NRF24_Driver.
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Definições de tipos existentes (mantidas)
typedef enum {
    COMM_ROBOT_TYPE_UNDEFINED,
    COMM_ROBOT_TYPE_SSL,
    COMM_ROBOT_TYPE_VSSS
} comm_robot_type_t;

typedef enum {
    COMM_NODE_MODE_TRANSMITTER,
    COMM_NODE_MODE_RECEIVER
} comm_node_mode_t;

// Tipos de Ponteiros de Função para Callbacks de Recepção (assinaturas mantidas,
// mas a lógica para obter robot_id_from_payload será diferente internamente no CommManager)
typedef void (*comm_ssl_packet_handler_t)(const ssl_payload_t* ssl_data, uint8_t robot_id_from_payload, uint8_t seq_num);
typedef void (*comm_vsss_packet_handler_t)(const vsss_payload_t* vsss_data, uint8_t robot_id_from_payload, uint8_t seq_num);
typedef void (*comm_debug_text_handler_t)(const char* text_data, uint8_t seq_num);

// **Alteração:** Nova estrutura para configurar o hardware de cada módulo NRF24L01+.
// Esta estrutura encapsula os pinos GPIO e o handle SPI para uma instância de NRF.
// Idealmente, esta estrutura poderia estar em NRF24_DEF.hpp ou em um novo arquivo de configuração de hardware.
struct NRF24_Hardware_Config {
    SPI_HandleTypeDef* hspi;     // Handle para a interface SPI (e.g., &hspi1)
    GPIO_TypeDef* ce_port;       // Porta GPIO para o pino CE (Chip Enable)
    uint16_t ce_pin;             // Pino GPIO para o CE
    GPIO_TypeDef* csn_port;      // Porta GPIO para o pino CSN (Chip Select Not)
    uint16_t csn_pin;            // Pino GPIO para o CSN
    uint32_t spi_timeout_ms;     // Tempo limite para operações SPI
};

// **Alteração:** Introdução da classe CommManager.
// Esta classe gerenciará as múltiplas instâncias de NRF24_Driver
// (um para cada módulo NRF físico) e a lógica de comunicação de alto nível.
class CommManager {
public:
    // Construtor da classe
    CommManager();

    // **Alteração:** Método de inicialização da comunicação.
    // Agora aceita configurações de hardware para múltiplos NRFs, dependendo do modo (Transmissor/Receptor).
    // Permite inicializar o CommManager com os NRFs de RX e/ou TX necessários.
    bool Init(comm_robot_type_t robot_type,
              comm_node_mode_t node_mode,
              uint8_t channel, // Canal RF comum para todos os NRFs (simplificação, pode ser específico por NRF se necessário)
              // Configurações de Hardware específicas:
              const NRF24_Hardware_Config* rx_nrf_hw_config, // Para o NRF de recepção (Robô RX, Torre RX Telemetria)
              const NRF24_Hardware_Config* tx_nrf_hw_config, // Para o NRF de transmissão (Robô TX)
              // Endereços de comunicação:
              uint8_t* base_tx_address,    // Endereço de TX padrão (ex: endereço da torre para robôs enviarem telemetria)
              uint8_t* robot_rx_address,   // Endereço de RX do próprio robô (somente para modo RECEIVER)
              uint8_t robot_id,             // ID do robô (para robôs definirem seu próprio endereço, e para torre saber qual robô enviar comando)
              // **Alteração:** Parâmetros específicos para o MODO TRANSMITTER (Torre):
              const NRF24_Hardware_Config command_tx_nrf_hw_configs[5], // Array de 5 configurações de HW para os NRFs de comando TX da Torre
              uint8_t robot_tx_addresses[5][5]); // Endereços de TX para cada robô (endereços de escuta dos robôs)

    // **Alteração:** Métodos de envio de mensagens agora recebem 'target_robot_id'.
    // Isso permite que o CommManager, no modo TRANSMITTER (Torre), selecione o NRF correto
    // para enviar a mensagem para o robô alvo. No modo RECEIVER (Robô), este parâmetro
    // pode ser ignorado ou usado para validação interna.
    bool Send_SSL_Message(uint8_t target_robot_id, const ssl_payload_t* ssl_payload_data);
    bool Send_VSSS_Message(uint8_t target_robot_id, const vsss_payload_t* vsss_payload_data);
    bool Send_DebugText_Message(uint8_t target_robot_id, const char* text_payload);

    // Métodos de registro de callback (assinaturas mantidas, implementações internas mudam)
    void Register_SSL_Packet_Handler(comm_ssl_packet_handler_t callback);
    void Register_VSSS_Packet_Handler(comm_vsss_packet_handler_t callback);
    void Register_DebugText_Packet_Handler(comm_debug_text_handler_t callback);

    // **Alteração:** Método para processar pacotes recebidos.
    // Agora este método verificará e lerá pacotes de todas as instâncias de NRF de recepção ativas
    // (ex: NRF de RX do robô, ou NRF de RX de telemetria da torre).
    void ProcessReceivedPackets(void);

private:
    comm_robot_type_t current_robot_type;
    comm_node_mode_t current_node_mode;
    uint8_t packet_seq_counter;
    comm_packet_t comm_packet_buffer; // Buffer para montagem/desmontagem de pacotes

    // **Alteração:** Ponteiros para instâncias da classe NRF24_Driver.
    // Permite flexibilidade na alocação e gerenciamento dos NRFs.
    NRF24_Driver* rx_nrf_instance;     // Instância do NRF para recepção (usado por Robô e Torre)
    NRF24_Driver* tx_nrf_instance;     // Instância do NRF para transmissão (usado apenas por Robô)

    // **Alteração:** Array de ponteiros para as 5 instâncias NRF24_Driver para transmissão de comandos da Torre.
    NRF24_Driver* command_tx_nrf_instances[5];

    // Variáveis para armazenar os endereços de comunicação
    uint8_t tower_base_rx_address[5]; // Endereço de escuta da torre (base para telemetria dos robôs)
    uint8_t robot_own_rx_address[5];  // Endereço de escuta do próprio robô
    // **Alteração:** Endereços de transmissão dos robôs para a torre (usados pela torre para RX de telemetria)
    uint8_t robot_tx_addresses_for_tower[5][5];
    // **Alteração:** Endereços de comando TX da torre para cada robô (endereços de escuta dos robôs)
    uint8_t command_tx_addresses_for_robots[5][5];

    // Ponteiros para as funções de callback registradas (mantidos)
    comm_ssl_packet_handler_t p_ssl_handler;
    comm_vsss_packet_handler_t p_vsss_handler;
    comm_debug_text_handler_t p_debug_text_handler;

    // **Alteração:** Método auxiliar privado para criar e inicializar uma instância de NRF24_Driver.
    // Isso centraliza a lógica de criação e configuração do driver de baixo nível.
    NRF24_Driver* createAndInitNRFDriver(const NRF24_Hardware_Config* hw_config, uint8_t channel,
                                        uint8_t* rx_addr_p0, uint8_t* rx_addr_p1, uint8_t rx_addr_p2_lsb,
                                        uint8_t* tx_addr);
};

// **Alteração:** Bloco extern "C" mantido para compatibilidade com arquivos .c (como main.c).
// Essas funções atuarão como wrappers para uma única instância global (singleton) do CommManager.
#ifdef __cplusplus
extern "C" {
#endif

// **Alteração:** As assinaturas das funções C-style foram atualizadas para refletir
// a nova necessidade de passar configurações de hardware e endereços para múltiplos NRFs.
bool Comm_Init(comm_robot_type_t robot_type,
               comm_node_mode_t node_mode,
               uint8_t channel,
               const NRF24_Hardware_Config* rx_nrf_hw_config,
               const NRF24_Hardware_Config* tx_nrf_hw_config,
               uint8_t* base_tx_address,
               uint8_t* robot_rx_address,
               uint8_t robot_id,
               const NRF24_Hardware_Config command_tx_nrf_hw_configs[5],
               uint8_t robot_tx_addresses[5][5]);

// **Alteração:** As funções de envio agora exigem o 'target_robot_id' para direcionamento.
bool Comm_Send_SSL_Message(uint8_t target_robot_id, const ssl_payload_t* ssl_payload_data);
bool Comm_Send_VSSS_Message(uint8_t target_robot_id, const vsss_payload_t* vsss_payload_data);
bool Comm_Send_DebugText_Message(uint8_t target_robot_id, const char* text_payload);

// Funções de registro de callback (assinaturas mantidas)
void Comm_Register_SSL_Packet_Handler(comm_ssl_packet_handler_t callback);
void Comm_Register_VSSS_Packet_Handler(comm_vsss_packet_handler_t callback);
void Comm_Register_DebugText_Packet_Handler(comm_debug_text_handler_t callback);

// Função para processar pacotes recebidos (assinatura mantida, implementação interna muda)
void Comm_ProcessReceivedPackets(void);

#ifdef __cplusplus
}
#endif

#endif /* COMM_HPP_ */