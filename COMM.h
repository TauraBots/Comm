
#ifndef COMM_H_
#define COMM_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "COMM_PACKETS.h" 

typedef enum {
    COMM_ROBOT_TYPE_UNDEFINED,
    COMM_ROBOT_TYPE_SSL,
    COMM_ROBOT_TYPE_VSSS
} comm_robot_type_t;

typedef void (*comm_ssl_command_handler_t)(const ssl_command_payload_t* cmd_data, uint8_t robot_id, uint8_t seq_num);
typedef void (*comm_vsss_command_handler_t)(const vsss_command_payload_t* cmd_data, uint8_t robot_id, uint8_t seq_num);
typedef void (*comm_ssl_telemetry_handler_t)(const ssl_telemetry_payload_t* tel_data, uint8_t robot_id, uint8_t seq_num);
typedef void (*comm_vsss_telemetry_handler_t)(const vsss_telemetry_payload_t* tel_data, uint8_t robot_id, uint8_t seq_num);
typedef void (*comm_debug_text_handler_t)(const char* text_data, uint8_t seq_num);


// -------------------- Funções da Interface do Módulo --------------------

/**
 * @brief Inicializa o módulo de comunicação P2P e o NRF24L01+.
 * @param robot_type O tipo de robô principal associado a esta instância.
 * @param channel Canal RF a ser usado (0-125).
 * @param my_listening_address Endereço de 5 bytes no qual este nó irá escutar por pacotes.
 * @param peer_target_address Endereço de 5 bytes do nó parceiro para o qual este nó irá transmitir.
 * @return true se a inicialização foi bem-sucedida, false caso contrário.
 */
bool Comm_Init_P2P(comm_robot_type_t robot_type,
                   uint8_t channel,
                   const uint8_t my_listening_address[5],
                   const uint8_t peer_target_address[5]);

// --- Funções de Envio ---
int16_t Comm_Send_SSL_Command(const ssl_command_payload_t* cmd_payload_data);
int16_t Comm_Send_VSSS_Command(const vsss_command_payload_t* cmd_payload_data);
int16_t Comm_Send_SSL_Telemetry(const ssl_telemetry_payload_t* tel_payload_data);
int16_t Comm_Send_VSSS_Telemetry(const vsss_telemetry_payload_t* tel_payload_data);
int16_t Comm_Send_DebugText_Message(const char* text_payload);


// --- Funções de Registro de Callback ---
void Comm_Register_SSL_CommandHandler(comm_ssl_command_handler_t callback);
void Comm_Register_VSSS_CommandHandler(comm_vsss_command_handler_t callback);
void Comm_Register_SSL_TelemetryHandler(comm_ssl_telemetry_handler_t callback);
void Comm_Register_VSSS_TelemetryHandler(comm_vsss_telemetry_handler_t callback);
void Comm_Register_DebugText_Packet_Handler(comm_debug_text_handler_t callback);

/**
 * @brief Processa mensagens NRF24L01+ recebidas.
 * Esta função deve ser chamada periodicamente no loop principal para verificar por pacotes.
 */
void Comm_ProcessReceivedPackets(void);

#endif /* COMM_H_ */