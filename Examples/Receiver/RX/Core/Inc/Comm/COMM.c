#include <Comm/COMM.h>
#include <Comm/COMM_PACKETS.h>
#include <Comm/NRF24_CORE.h>
#include <string.h>
#include <stdio.h>


static comm_robot_type_t    s_current_robot_type;
static comm_node_mode_t     s_current_node_mode;
static uint8_t              s_packet_seq_counter = 0;
static comm_packet_t        s_comm_packet_buffer;


static comm_ssl_packet_handler_t  s_ssl_handler = NULL;
static comm_vsss_packet_handler_t s_vsss_handler = NULL;
static comm_debug_text_handler_t  s_debug_text_handler = NULL;


bool Comm_Init(comm_robot_type_t robot_type,
               comm_node_mode_t node_mode,
               uint8_t channel,
               uint8_t* nrf_tx_address,
               uint8_t* nrf_rx_pipe1_address,
               uint8_t nrf_rx_pipe2_lsb)
{
    s_current_robot_type = robot_type;
    s_current_node_mode = node_mode;
    s_packet_seq_counter = 0;

    printf("Comm_Init: Modo %s, Robô %s, Canal %d\r\n",
        (node_mode == COMM_NODE_MODE_TRANSMITTER) ? "TX" : "RX",
        (robot_type == COMM_ROBOT_TYPE_SSL) ? "SSL" : (robot_type == COMM_ROBOT_TYPE_VSSS) ? "VSSS" : "N/A",
        channel);

    NRF24_Init();

    if (s_current_node_mode == COMM_NODE_MODE_TRANSMITTER) {
        NRF24_TxMode(nrf_tx_address, channel);
        printf("NRF24 configurado como Transmissor.\r\n");
    } else {
        NRF24_RxMode(nrf_rx_pipe1_address, nrf_rx_pipe2_lsb, channel);
        printf("NRF24 configurado como Receptor.\r\n");
    }
    return true;
}


bool Comm_Send_SSL_Message(const ssl_payload_t* ssl_payload_data) {
    if (s_current_node_mode != COMM_NODE_MODE_TRANSMITTER) {
        printf("Comm_Send_SSL_Message: Erro - Nó não está em modo transmissor.\r\n");
        return false;
    }
    if (ssl_payload_data == NULL) {
        printf("Comm_Send_SSL_Message: Erro - Payload SSL nulo.\r\n");
        return false;
    }

    Comm_Packets_Create_SSLMessage(&s_comm_packet_buffer, s_packet_seq_counter++, ssl_payload_data);

    //printf("Comm Enviando SSL: Seq %d, ID %d, Vx %d\r\n", s_comm_packet_buffer.header.seq_number, ssl_payload_data->robot_id, ssl_payload_data->vx);
    return NRF24_Transmit((uint8_t*)&s_comm_packet_buffer, sizeof(comm_packet_t));
}

bool Comm_Send_VSSS_Message(const vsss_payload_t* vsss_payload_data) {
    if (s_current_node_mode != COMM_NODE_MODE_TRANSMITTER) {
        printf("Comm_Send_VSSS_Message: Erro - Nó não está em modo transmissor.\r\n");
        return false;
    }
    if (vsss_payload_data == NULL) {
        printf("Comm_Send_VSSS_Message: Erro - Payload VSSS nulo.\r\n");
        return false;
    }

    Comm_Packets_Create_VSSSMessage(&s_comm_packet_buffer, s_packet_seq_counter++, vsss_payload_data);

    // printf("Comm Enviando VSSS: Seq %d, ID %d, M1 %d\r\n", s_comm_packet_buffer.header.seq_number, vsss_payload_data->robot_id, vsss_payload_data->motor1_value);
    return NRF24_Transmit((uint8_t*)&s_comm_packet_buffer, sizeof(comm_packet_t));
}

bool Comm_Send_DebugText_Message(const char* text_payload) {
    if (s_current_node_mode != COMM_NODE_MODE_TRANSMITTER) {
        printf("Comm_Send_DebugText_Message: Erro - Nó não está em modo transmissor.\r\n");
        return false;
    }
    if (text_payload == NULL) {
        printf("Comm_Send_DebugText_Message: Erro - Texto nulo.\r\n");
        return false;
    }

    Comm_Packets_Create_DebugText(&s_comm_packet_buffer, s_packet_seq_counter++, text_payload);

    // printf("Comm Enviando Debug: Seq %d, Texto: %s\r\n", s_comm_packet_buffer.header.seq_number, text_payload);
    return NRF24_Transmit((uint8_t*)&s_comm_packet_buffer, sizeof(comm_packet_t));
}



void Comm_Register_SSL_Packet_Handler(comm_ssl_packet_handler_t callback) {
    s_ssl_handler = callback;
}

void Comm_Register_VSSS_Packet_Handler(comm_vsss_packet_handler_t callback) {
    s_vsss_handler = callback;
}

void Comm_Register_DebugText_Packet_Handler(comm_debug_text_handler_t callback) {
    s_debug_text_handler = callback;
}

void Comm_ProcessReceivedPackets(void) {
    if (s_current_node_mode != COMM_NODE_MODE_RECEIVER) {
        return;
    }

    uint8_t raw_input_buffer[NRF_MAX_PACKET_SIZE];

    if (isDataAvailable(1)) { // Verifica Pipe 1 (poderia ser configurável ou verificar múltiplos pipes)
        NRF24_Receive(raw_input_buffer);

        memcpy(&s_comm_packet_buffer, raw_input_buffer, sizeof(comm_packet_t));

        // HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin); // Feedback visual pode ir para o main ou callback

        // printf("Comm Pacote Recebido! Tipo: %d, Seq: %d\r\n",
        //        s_comm_packet_buffer.header.main_type,
        //        s_comm_packet_buffer.header.seq_number);

        switch (s_comm_packet_buffer.header.main_type) {
            case MAIN_PACKET_TYPE_SSL_MESSAGE:
                if (s_ssl_handler != NULL) {
                    s_ssl_handler(&s_comm_packet_buffer.payload_u.ssl_msg,
                                  s_comm_packet_buffer.payload_u.ssl_msg.robot_id, // Passando o ID do payload
                                  s_comm_packet_buffer.header.seq_number);
                }
                break;

            case MAIN_PACKET_TYPE_VSSS_MESSAGE:
                if (s_vsss_handler != NULL) {
                    s_vsss_handler(&s_comm_packet_buffer.payload_u.vsss_msg,
                                   s_comm_packet_buffer.payload_u.vsss_msg.robot_id, // Passando o ID do payload
                                   s_comm_packet_buffer.header.seq_number);
                }
                break;

            case MAIN_PACKET_TYPE_DEBUG_TEXT:
                if (s_debug_text_handler != NULL) {
                    char temp_text[DEBUG_TEXT_MAX_LEN + 1];
                    memcpy(temp_text, s_comm_packet_buffer.payload_u.debug_text.text, DEBUG_TEXT_MAX_LEN);
                    temp_text[DEBUG_TEXT_MAX_LEN] = '\0';
                    s_debug_text_handler(temp_text, s_comm_packet_buffer.header.seq_number);
                }
                break;

            default:
                printf("Comm: Tipo de pacote principal desconhecido: %d\r\n", s_comm_packet_buffer.header.main_type);
                break;
        }
    }
}
