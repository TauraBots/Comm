#ifndef COMM_PACKETS_H_
#define COMM_PACKETS_H_

#include <stdint.h>
#include <stdbool.h>


#define NRF_MAX_PACKET_SIZE 32

typedef enum {
    MAIN_PACKET_TYPE_UNDEFINED = 0,
    MAIN_PACKET_TYPE_VSSS_MESSAGE,
    MAIN_PACKET_TYPE_SSL_MESSAGE,
    MAIN_PACKET_TYPE_DEBUG_TEXT,

} nrf_main_packet_type_t;


typedef enum {
    VSSS_CMD_SUBTYPE_UNDEFINED = 0,
    VSSS_CMD_SET_MOTOR_SPEEDS,
    VSSS_CMD_RESEVED_VSSS,

} vsss_command_subtype_t;


typedef enum {
    SSL_CMD_SUBTYPE_UNDEFINED = 0,
    SSL_CMD_SET_VELOCITIES,
    SSL_CMD_ACTION_CONTROL,
    SSL_CMD_REFEREE,
} ssl_command_subtype_t;


typedef struct __attribute__((packed)) {
    vsss_command_subtype_t command_subtype;
    uint8_t  robot_id;                      // ID do robô (0-255)
    uint16_t motor1_value;                  // Valor para o motor 1
    uint16_t motor2_value;                  // Valor para o motor 2
    uint8_t  is_pwm_flag;                   // 1 se os valores dos motores são PWM, 0 se RPM
} vsss_payload_t;                           // Tamanho: 1 + 1 + 2 + 2 + 1 = 7 bytes


typedef struct __attribute__((packed)) {
    ssl_command_subtype_t command_subtype;  // Subtipo do comando SSL
    uint8_t  robot_id;                      // ID do robô
    int16_t  vx;                            // Velocidade X (mm/s * 10 ou outra escala)
    int16_t  vy;                            // Velocidade Y (mm/s * 10 ou outra escala)
    int16_t  vw;                            // Velocidade angular (rad/s * 10000 ou outra escala)
    uint8_t  referee_command;               // Comando de jogo (refereeCommand)
    uint8_t  kick_front;                    // Chute frontal (0=não, 1=sim)
    uint8_t  kick_chip;                     // Chute em arco (0=não, 1=sim)
    uint8_t  capacitor_charge;              // Acionar capacitor (0=não, 1=sim)
    uint8_t  kick_strength;                 // Força do chute (0–255 ou escalado)
    uint8_t  dribbler_on;                   // Liga dribbler (0=não, 1=sim)
    uint8_t  dribbler_speed;                // Velocidade do dribbler
    uint8_t  movement_locked;               // Proibir movimento (0=não, 1=sim)
    uint8_t  critical_move_turbo;           // Ativar turbo (0=não, 1=sim)
} ssl_payload_t;                            // Tamanho: 1+1+2+2+2+1 + 1+1+1+1+1+1+1+1 = 17 bytes


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
        vsss_payload_t vsss_msg;
        ssl_payload_t  ssl_msg;
        debug_text_payload_t debug_text;
    } payload_u;

} comm_packet_t;


void Comm_Packets_Create_VSSSMessage(comm_packet_t* packet_buffer,
                                     uint8_t seq_num,
                                     const vsss_payload_t* vsss_payload_data);


void Comm_Packets_Create_SSLMessage(comm_packet_t* packet_buffer,
                                    uint8_t seq_num,
                                    const ssl_payload_t* ssl_payload_data);

void Comm_Packets_Create_DebugText(comm_packet_t* packet_buffer,
                                   uint8_t seq_num,
                                   const char* text_payload);

#endif
