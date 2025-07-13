// Comm/COMM_PACKETS.cpp
#include <Comm/COMM_PACKETS.hpp> // Atualizado caminho de inclusão
#include <string.h>

void Comm_Packets_Create_VSSSMessage(comm_packet_t* packet_buffer,
                                     uint8_t seq_num,
                                     const vsss_payload_t* vsss_payload_data)
{
    if (packet_buffer == nullptr || vsss_payload_data == nullptr) { // Opcional: usar nullptr em vez de NULL
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_VSSS_MESSAGE;
    packet_buffer->header.seq_number = seq_num;

    memcpy(&packet_buffer->payload_u.vsss_msg, vsss_payload_data, sizeof(vsss_payload_t));
}

void Comm_Packets_Create_SSLMessage(comm_packet_t* packet_buffer,
                                    uint8_t seq_num,
                                    const ssl_payload_t* ssl_payload_data)
{
    if (packet_buffer == nullptr || ssl_payload_data == nullptr) { // Opcional: usar nullptr em vez de NULL
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_SSL_MESSAGE;
    packet_buffer->header.seq_number = seq_num;

    memcpy(&packet_buffer->payload_u.ssl_msg, ssl_payload_data, sizeof(ssl_payload_t));
}

void Comm_Packets_Create_DebugText(comm_packet_t* packet_buffer,
                                   uint8_t seq_num,
                                   const char* text_payload)
{
    if (packet_buffer == nullptr || text_payload == nullptr) { // Opcional: usar nullptr em vez de NULL
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_DEBUG_TEXT;
    packet_buffer->header.seq_number = seq_num;

    strncpy(packet_buffer->payload_u.debug_text.text, text_payload, DEBUG_TEXT_MAX_LEN -1);
    packet_buffer->payload_u.debug_text.text[DEBUG_TEXT_MAX_LEN - 1] = '\0';
}