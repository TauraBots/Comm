
#include "Comm/COMM.h"
#include "Comm/NRF24_CORE.h"
#include "Comm/COMM_PACKETS.h"
#include <string.h>
#include <stdio.h>

static comm_robot_type_t    s_current_robot_type;
static uint8_t              s_packet_seq_counter = 0;
static comm_packet_t        s_tx_packet_buffer;
static comm_packet_t        s_rx_packet_buffer;

static uint8_t s_my_listening_address[5];
static uint8_t s_peer_target_address[5];
static uint8_t s_rf_channel;

static comm_ssl_command_handler_t    s_ssl_cmd_handler = NULL;
static comm_vsss_command_handler_t   s_vsss_cmd_handler = NULL;
static comm_ssl_telemetry_handler_t  s_ssl_tel_handler = NULL;
static comm_vsss_telemetry_handler_t s_vsss_tel_handler = NULL;
static comm_debug_text_handler_t     s_debug_text_handler = NULL;

static bool send_packet_p2p(comm_packet_t* packet_to_send) {
    NRF24_TxMode(s_peer_target_address, s_rf_channel);
    bool success = NRF24_Transmit((uint8_t*)packet_to_send, sizeof(comm_packet_t));
    NRF24_RxMode(s_my_listening_address, 0, s_rf_channel); 
    
    return success;
}

bool Comm_Init_P2P(comm_robot_type_t robot_type,
                   uint8_t channel,
                   const uint8_t my_listening_address[5],
                   const uint8_t peer_target_address[5]) {
    s_current_robot_type = robot_type;
    s_packet_seq_counter = 0;
    s_rf_channel = channel;

    memcpy(s_my_listening_address, my_listening_address, 5);
    memcpy(s_peer_target_address, peer_target_address, 5);

    printf("Comm_Init_P2P: Robô tipo %d, Canal %d\r\n", robot_type, channel);

    NRF24_Init();
    NRF24_RxMode(s_my_listening_address, 0, s_rf_channel); // Inicia escutando
    printf("NRF24 configurado como Transceptor, iniciando em modo RX.\r\n");
    return true;
}

// --- Funções de Envio ---
int16_t Comm_Send_SSL_Command(const ssl_command_payload_t* cmd_payload_data) {
    if (cmd_payload_data == NULL) return -1;
    uint8_t current_seq = s_packet_seq_counter++;
    Comm_Packets_Create_SSLCommand(&s_tx_packet_buffer, current_seq, cmd_payload_data);
    if (send_packet_p2p(&s_tx_packet_buffer)) {
        return current_seq;
    }
    return -1; 
}
int16_t Comm_Send_VSSS_Command(const vsss_command_payload_t* cmd_payload_data) {
    if (cmd_payload_data == NULL) return -1;
    uint8_t current_seq = s_packet_seq_counter++;
    Comm_Packets_Create_VSSSCommand(&s_tx_packet_buffer, current_seq, cmd_payload_data);
    if (send_packet_p2p(&s_tx_packet_buffer)) {
        return current_seq;
    }
    return -1;
}

int16_t Comm_Send_SSL_Telemetry(const ssl_telemetry_payload_t* tel_payload_data) {
    if (tel_payload_data == NULL) return -1;
    uint8_t current_seq = s_packet_seq_counter++;
    Comm_Packets_Create_SSLTelemetry(&s_tx_packet_buffer, current_seq, tel_payload_data);
    if (send_packet_p2p(&s_tx_packet_buffer)) {
        return current_seq;
    }
    return -1;
}

int16_t Comm_Send_VSSS_Telemetry(const vsss_telemetry_payload_t* tel_payload_data) {
    if (tel_payload_data == NULL) return -1;
    uint8_t current_seq = s_packet_seq_counter++;
    Comm_Packets_Create_VSSTelemetry(&s_tx_packet_buffer, current_seq, tel_payload_data);
    if (send_packet_p2p(&s_tx_packet_buffer)) {
        return current_seq;
    }
    return -1;
}
int16_t Comm_Send_DebugText_Message(const char* text_payload) { 
    if (text_payload == NULL) return -1; 
    uint8_t current_seq = s_packet_seq_counter++;
    Comm_Packets_Create_DebugText(&s_tx_packet_buffer, current_seq, text_payload);
    if (send_packet_p2p(&s_tx_packet_buffer)) {
        return current_seq; 
    }
    return -1;
}

void Comm_Register_SSL_CommandHandler(comm_ssl_command_handler_t callback) { s_ssl_cmd_handler = callback; }
void Comm_Register_VSSS_CommandHandler(comm_vsss_command_handler_t callback) { s_vsss_cmd_handler = callback; }
void Comm_Register_SSL_TelemetryHandler(comm_ssl_telemetry_handler_t callback) { s_ssl_tel_handler = callback; }
void Comm_Register_VSSS_TelemetryHandler(comm_vsss_telemetry_handler_t callback) { s_vsss_tel_handler = callback; }
void Comm_Register_DebugText_Packet_Handler(comm_debug_text_handler_t callback) { s_debug_text_handler = callback; }

void Comm_ProcessReceivedPackets(void) {
    if (isDataAvailable(1)) {
        uint8_t raw_input_buffer[NRF_MAX_PACKET_SIZE];
        NRF24_Receive(raw_input_buffer);
        memcpy(&s_rx_packet_buffer, raw_input_buffer, sizeof(comm_packet_t));

        switch (s_rx_packet_buffer.header.main_type) {
            case MAIN_PACKET_TYPE_SSL_COMMAND:
                if (s_ssl_cmd_handler != NULL) s_ssl_cmd_handler(&s_rx_packet_buffer.payload_u.ssl_cmd, s_rx_packet_buffer.payload_u.ssl_cmd.robot_id, s_rx_packet_buffer.header.seq_number);
                break;
            case MAIN_PACKET_TYPE_VSSS_COMMAND:
                if (s_vsss_cmd_handler != NULL) s_vsss_cmd_handler(&s_rx_packet_buffer.payload_u.vsss_cmd, s_rx_packet_buffer.payload_u.vsss_cmd.robot_id, s_rx_packet_buffer.header.seq_number);
                break;
            case MAIN_PACKET_TYPE_SSL_TELEMETRY:
                if (s_ssl_tel_handler != NULL) s_ssl_tel_handler(&s_rx_packet_buffer.payload_u.ssl_tel, s_rx_packet_buffer.payload_u.ssl_tel.robot_id, s_rx_packet_buffer.header.seq_number);
                break;
            case MAIN_PACKET_TYPE_VSSS_TELEMETRY:
                if (s_vsss_tel_handler != NULL) s_vsss_tel_handler(&s_rx_packet_buffer.payload_u.vsss_tel, s_rx_packet_buffer.payload_u.vsss_tel.robot_id, s_rx_packet_buffer.header.seq_number);
                break;
            case MAIN_PACKET_TYPE_DEBUG_TEXT:
                break;
            default:
                printf("Comm: Tipo de pacote principal desconhecido: %d\r\n", s_rx_packet_buffer.header.main_type);
                break;
        }
    }
}