#include <Comm/COMM_PACKETS.h>
#include <string.h>


void Comm_Packets_Create_VSSSCommand(comm_packet_t* packet_buffer,
                                     uint8_t seq_num,
                                     const vsss_command_payload_t* cmd_payload)
{
    if (packet_buffer == NULL || cmd_payload == NULL) {
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_VSSS_COMMAND;
    packet_buffer->header.seq_number = seq_num;

    memcpy(&packet_buffer->payload_u.vsss_cmd, cmd_payload, sizeof(vsss_command_payload_t));
}

void Comm_Packets_Create_SSLCommand(comm_packet_t* packet_buffer,
                                    uint8_t seq_num,
                                    const ssl_command_payload_t* cmd_payload)
{
    if (packet_buffer == NULL || cmd_payload == NULL) {
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_SSL_COMMAND;
    packet_buffer->header.seq_number = seq_num;

    memcpy(&packet_buffer->payload_u.ssl_cmd, cmd_payload, sizeof(ssl_command_payload_t));
}

void Comm_Packets_Create_DebugText(comm_packet_t* packet_buffer,
                                   uint8_t seq_num,
                                   const char* text_payload)
{
    if (packet_buffer == NULL || text_payload == NULL) {
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_DEBUG_TEXT;
    packet_buffer->header.seq_number = seq_num;

    strncpy(packet_buffer->payload_u.debug_text.text, text_payload, DEBUG_TEXT_MAX_LEN -1);
    packet_buffer->payload_u.debug_text.text[DEBUG_TEXT_MAX_LEN - 1] = '\0';
}


void Comm_Packets_Create_VSSTelemetry(comm_packet_t* packet_buffer,
                                      uint8_t seq_num,
                                      const vsss_telemetry_payload_t* tel_payload)
{
    if (packet_buffer == NULL || tel_payload == NULL) {
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_VSSS_TELEMETRY;
    packet_buffer->header.seq_number = seq_num;

    memcpy(&packet_buffer->payload_u.vsss_tel, tel_payload, sizeof(vsss_telemetry_payload_t));
}

void Comm_Packets_Create_SSLTelemetry(comm_packet_t* packet_buffer,
                                      uint8_t seq_num,
                                      const ssl_telemetry_payload_t* tel_payload)
{
    if (packet_buffer == NULL || tel_payload == NULL) {
        return;
    }
    memset(packet_buffer, 0, sizeof(comm_packet_t));

    packet_buffer->header.main_type = MAIN_PACKET_TYPE_SSL_TELEMETRY;
    packet_buffer->header.seq_number = seq_num;

    memcpy(&packet_buffer->payload_u.ssl_tel, tel_payload, sizeof(ssl_telemetry_payload_t));
}