#ifndef COMM_H_
#define COMM_H_

#include <Comm/COMM_PACKETS.h>
#include <Comm/NRF24_CORE.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    COMM_ROBOT_TYPE_UNDEFINED,
    COMM_ROBOT_TYPE_SSL,
    COMM_ROBOT_TYPE_VSSS
} comm_robot_type_t;

typedef enum {
    COMM_NODE_MODE_TRANSMITTER,
    COMM_NODE_MODE_RECEIVER
} comm_node_mode_t;

// Tipos de Ponteiros de Função para Callbacks de Recepção
typedef void (*comm_ssl_packet_handler_t)(const ssl_payload_t* ssl_data, uint8_t robot_id_from_payload, uint8_t seq_num);
typedef void (*comm_vsss_packet_handler_t)(const vsss_payload_t* vsss_data, uint8_t robot_id_from_payload, uint8_t seq_num);
typedef void (*comm_debug_text_handler_t)(const char* text_data, uint8_t seq_num);

// -------------------- Funções de Interface do Módulo de Comunicação --------------------

/**
 * @brief Inicializa o módulo de comunicação e o NRF24L01+.
 *
 * @param robot_type O tipo de robô principal para esta instância (COMM_ROBOT_TYPE_SSL ou COMM_ROBOT_TYPE_VSSS).
 * Pode influenciar filtros ou lógicas específicas no futuro.
 * @param node_mode  Define se o nó operará como COMM_NODE_MODE_TRANSMITTER ou COMM_NODE_MODE_RECEIVER.
 * @param channel    Canal RF a ser usado (0-125).
 * @param tx_address Endereço de 5 bytes para transmissão (se transmissor) ou para o Pipe 0 (se receptor, para ACKs).
 * @param rx_pipe1_address Endereço de 5 bytes para o Pipe 1 de recepção (se receptor).
 * @param rx_pipe2_lsb LSB para o Pipe 2 de recepção (se receptor, opcional, pode ser 0 se não usado).
 * @return true se a inicialização foi bem-sucedida, false caso contrário.
 */
bool Comm_Init(comm_robot_type_t robot_type,
               comm_node_mode_t node_mode,
               uint8_t channel,
               uint8_t* nrf_tx_address,
               uint8_t* nrf_rx_pipe1_address,
               uint8_t nrf_rx_pipe2_lsb); // Adicionado nrf_rx_pipe2_lsb

// --- Funções para Transmissor ---

/**
 * @brief Envia uma mensagem de controle para um robô SSL.
 * Apenas funciona se Comm_Init foi chamado com COMM_NODE_MODE_TRANSMITTER.
 * @param ssl_payload_data Ponteiro para a estrutura de payload SSL a ser enviada.
 * @return true se a transmissão foi enfileirada/iniciada com sucesso (ACK pendente), false caso contrário.
 */
bool Comm_Send_SSL_Message(const ssl_payload_t* ssl_payload_data);

/**
 * @brief Envia uma mensagem de controle para um robô VSSS.
 * Apenas funciona se Comm_Init foi chamado com COMM_NODE_MODE_TRANSMITTER.
 * @param vsss_payload_data Ponteiro para a estrutura de payload VSSS a ser enviada.
 * @return true se a transmissão foi enfileirada/iniciada com sucesso (ACK pendente), false caso contrário.
 */
bool Comm_Send_VSSS_Message(const vsss_payload_t* vsss_payload_data);

/**
 * @brief Envia uma mensagem de texto para debug.
 * Apenas funciona se Comm_Init foi chamado com COMM_NODE_MODE_TRANSMITTER.
 * @param text_payload String de texto a ser enviada.
 * @return true se a transmissão foi enfileirada/iniciada com sucesso, false caso contrário.
 */
bool Comm_Send_DebugText_Message(const char* text_payload);


// --- Funções para Receptor ---

/**
 * @brief Registra uma função de callback para ser chamada quando um pacote SSL é recebido.
 * Apenas relevante se Comm_Init foi chamado com COMM_NODE_MODE_RECEIVER.
 * @param callback Ponteiro para a função handler. Passe NULL para desregistrar.
 */
void Comm_Register_SSL_Packet_Handler(comm_ssl_packet_handler_t callback);

/**
 * @brief Registra uma função de callback para ser chamada quando um pacote VSSS é recebido.
 * Apenas relevante se Comm_Init foi chamado com COMM_NODE_MODE_RECEIVER.
 * @param callback Ponteiro para a função handler. Passe NULL para desregistrar.
 */
void Comm_Register_VSSS_Packet_Handler(comm_vsss_packet_handler_t callback);

/**
 * @brief Registra uma função de callback para ser chamada quando um pacote de Debug Text é recebido.
 * Apenas relevante se Comm_Init foi chamado com COMM_NODE_MODE_RECEIVER.
 * @param callback Ponteiro para a função handler. Passe NULL para desregistrar.
 */
void Comm_Register_DebugText_Packet_Handler(comm_debug_text_handler_t callback);

/**
 * @brief Processa mensagens NRF24L01+ recebidas.
 * Esta função deve ser chamada periodicamente no loop principal do receptor.
 * Ela verifica se há dados, os recebe, interpreta o tipo e chama o callback apropriado.
 */
void Comm_ProcessReceivedPackets(void);


#endif /* COMM_H_ */
