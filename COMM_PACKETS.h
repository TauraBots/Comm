#ifndef COMM_PACKETS_H_
#define COMM_PACKETS_H_

#include <stdint.h>


#define NRF_MAX_PACKET_SIZE 32

typedef enum {
    MAIN_PACKET_TYPE_UNDEFINED = 0,
    MAIN_PACKET_TYPE_VSSS_COMMAND,     
    MAIN_PACKET_TYPE_SSL_COMMAND,     
    MAIN_PACKET_TYPE_VSSS_TELEMETRY,   
    MAIN_PACKET_TYPE_SSL_TELEMETRY,    
    MAIN_PACKET_TYPE_DEBUG_TEXT,
} nrf_main_packet_type_t;


typedef enum {
    VSSS_CMD_SUBTYPE_UNDEFINED = 0,
    VSSS_CMD_SET_MOTOR_SPEEDS,
    VSSS_CMD_RESERVED, 
} vsss_command_subtype_t;

typedef enum {
    SSL_CMD_SUBTYPE_UNDEFINED = 0,
    SSL_CMD_SET_VELOCITIES,
    SSL_CMD_ACTION_CONTROL,
    SSL_CMD_REFEREE,
} ssl_command_subtype_t;


typedef enum {
    VSSS_TELEMETRY_SUBTYPE_STANDARD = 0,
} vsss_telemetry_subtype_t;

typedef enum {
    SSL_TELEMETRY_SUBTYPE_STANDARD = 0,
} ssl_telemetry_subtype_t;



typedef struct __attribute__((packed)) {
    vsss_command_subtype_t command_subtype;
    uint8_t  robot_id;
    uint16_t motor1_value;
    uint16_t motor2_value;
    uint8_t  is_pwm_flag;
} vsss_command_payload_t; 

typedef struct __attribute__((packed)) {
    ssl_command_subtype_t command_subtype;
    uint8_t  robot_id;
    int16_t  vx;
    int16_t  vy;
    int16_t  vw;
    uint8_t  referee_command;
    uint8_t  kick_front;
    uint8_t  kick_chip;
    uint8_t  capacitor_charge;
    uint8_t  kick_strength;
    uint8_t  dribbler_on;
    uint8_t  dribbler_speed;
    uint8_t  movement_locked;
    uint8_t  critical_move_turbo;
} ssl_command_payload_t; 



typedef struct __attribute__((packed)) {
  uint8_t telemetry_subtype : 4;
  uint8_t robot_id : 4;
  int32_t motor1_feedback : 18;
  int32_t motor2_feedback : 18;
  uint8_t battery_level : 8;
  uint8_t reserved_bits : 4;
  uint8_t command_seq_num_ack;
} vsss_telemetry_payload_t;

typedef struct __attribute__((packed)) {
  uint8_t telemetry_subtype : 4;
  uint8_t robot_id : 4;
  uint16_t current_m1 : 12;
  uint16_t current_m2 : 12;
  uint16_t current_m3 : 12;
  uint16_t current_m4 : 12;
  int16_t dribbler_speed_feedback : 15;
  uint8_t kick_load_status : 8;
  uint8_t ball_detected : 1;
  uint8_t battery_level : 8;
  int16_t speed_m1_feedback : 16;
  int16_t speed_m2_feedback : 16;
  int16_t speed_m3_feedback : 16;
  int16_t speed_m4_feedback : 16;
  uint8_t telemetry_pckt_count : 8;
  uint8_t command_seq_num_ack;
} ssl_telemetry_payload_t;


// Payload de Debug
#define DEBUG_TEXT_MAX_LEN (NRF_MAX_PACKET_SIZE - 2)
typedef struct __attribute__((packed)) {
    char text[DEBUG_TEXT_MAX_LEN];
} debug_text_payload_t;


typedef struct __attribute__((packed)) {
    nrf_main_packet_type_t main_type;
    uint8_t                seq_number;
} comm_packet_header_t;

#define COMM_USER_PAYLOAD_MAX_SIZE (NRF_MAX_PACKET_SIZE - sizeof(comm_packet_header_t))

typedef struct __attribute__((packed)) {
    comm_packet_header_t header;
    union {
        uint8_t raw_payload[COMM_USER_PAYLOAD_MAX_SIZE];
        vsss_command_payload_t vsss_cmd;
        ssl_command_payload_t  ssl_cmd;
        vsss_telemetry_payload_t vsss_tel;
        ssl_telemetry_payload_t  ssl_tel;
        debug_text_payload_t debug_text;
    } payload_u;
} comm_packet_t;



void Comm_Packets_Create_VSSSCommand(comm_packet_t* packet_buffer, uint8_t seq_num, const vsss_command_payload_t* cmd_payload);
void Comm_Packets_Create_SSLCommand(comm_packet_t* packet_buffer, uint8_t seq_num, const ssl_command_payload_t* cmd_payload);
void Comm_Packets_Create_VSSTelemetry(comm_packet_t* packet_buffer, uint8_t seq_num, const vsss_telemetry_payload_t* tel_payload);
void Comm_Packets_Create_SSLTelemetry(comm_packet_t* packet_buffer, uint8_t seq_num, const ssl_telemetry_payload_t* tel_payload);
void Comm_Packets_Create_DebugText(comm_packet_t* packet_buffer, uint8_t seq_num, const char* text_payload);


#endif /* COMM_PACKETS_H_ */