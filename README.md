# Biblioteca NRF24L01+ para STM32 com Sistema de Pacotes Customizado para `SSL` e `VSSS` (Portado para C++)

Esta biblioteca fornece uma interface para comunicação sem fio utilizando o módulo NRF24L01+ com microcontroladores STM32, utilizando a camada HAL da ST. Inclui um sistema para gerenciamento de pacotes customizados, permitindo a transmissão de diferentes tipos de dados estruturados, como comandos para robôs VSSS e SSL. Esta versão foi portada da linguagem C para C++.

## Visão Geral

A biblioteca é modularizada em:
* **Camada de Definições (`NRF24_DEF.hpp`):** Constantes, definições de pinos, registradores do NRF24L01+.
* **Camada de Abstração de Hardware (`NRF24_HAL.cpp/hpp`):** Funções de baixo nível para controle de pinos (CE, CSN) e comunicação SPI, utilizando as funções HAL do STM32.
* **Núcleo do Driver NRF24 (`NRF24_CORE.cpp/hpp`):** Lógica principal para operar o NRF24L01+, incluindo inicialização, configuração de modos (TX/RX), envio e recepção de dados.
* **Gerenciamento de Pacotes (`COMM_PACKETS.cpp/hpp`):** Definição de estruturas de pacotes, tipos de mensagens e funções auxiliares para criar e interpretar pacotes específicos para diferentes aplicações (ex: VSSS, SSL).
* **Interface de Comunicação (`COMM.cpp/hpp`):** Camada de alto nível para inicialização e gerenciamento da comunicação, encapsulando as funcionalidades do NRF24 e dos pacotes.

## Pré-requisitos

### Hardware
* Microcontrolador STM32 (ex: STM32F411xE, STM32G4xxx, STM32H7xxx)
* Módulo NRF24L01+
* Conexões SPI entre o STM32 e o NRF24L01+ (MOSI, MISO, SCK, CSN)
* Conexões GPIO para os pinos CE do NRF24L01+
* (Opcional) LED para debug visual
* (Opcional) Conversor USB-Serial para `printf` via UART

### Software
* Ambiente de desenvolvimento STM32 (ex: STM32CubeIDE)
* Bibliotecas STM32 HAL
* Compilador C++ (g++ ou similar)

## Estrutura de Arquivos da Biblioteca

Assumindo que os arquivos da biblioteca estão em uma subpasta `Comm` dentro da pasta de includes e fontes do seu projeto (ex: `Core/Inc/Comm/`):

* `Core/Inc/Comm/NRF24_DEF.hpp`: Definições de hardware, registradores e constantes do NRF24.
* `Core/Inc/Comm/NRF24_HAL.hpp`: Protótipos para a camada de abstração de hardware.
* `Core/Inc/Comm/NRF24_HAL.cpp`: Implementações da camada de abstração de hardware.
* `Core/Inc/Comm/NRF24_CORE.hpp`: Protótipos para o núcleo do driver NRF24.
* `Core/Inc/Comm/NRF24_CORE.cpp`: Implementações do núcleo do driver NRF24.
* `Core/Inc/Comm/COMM_PACKETS.hpp`: Definições de tipos de pacotes, subtipos e estruturas de payload.
* `Core/Inc/Comm/COMM_PACKETS.cpp`: Funções auxiliares para criar pacotes.
* `Core/Inc/Comm/COMM.hpp`: Protótipos para a interface de comunicação de alto nível.
* `Core/Inc/Comm/COMM.cpp`: Implementações da interface de comunicação de alto nível.

## Configuração

### 1. Configuração de Pinos e SPI (`NRF24_DEF.hpp`)

Edite o arquivo `NRF24_DEF.hpp` para corresponder à sua configuração de hardware:

```cpp
// NRF24_DEF.hpp

// Definições de pinos NRF24 - Adapte conforme sua placa
#include "stm32f4xx_hal.h"

#define NRF24_CE_PORT   GPIOB      // Porta do pino CE (ex: GPIOB)
#define NRF24_CE_PIN    GPIO_PIN_1 // Pino CE (ex: GPIO_PIN_1)

#define NRF24_CSN_PORT  GPIOB      // Porta do pino CSN (ex: GPIOB)
#define NRF24_CSN_PIN   GPIO_PIN_0 // Pino CSN (ex: GPIO_PIN_0)

// Handle SPI (deve ser o mesmo definido e inicializado em main.cpp)
extern SPI_HandleTypeDef hspi1; // Mude hspi1 se seu handle SPI for diferente
#define NRF24_SPI       &hspi1
```
Importante: Assegure-se de que os pinos CE_Pin e CSN_Pin configurados no CubeMX (e definidos em main.h) correspondem aos NRF24_CE_PIN e NRF24_CSN_PIN em NRF24_DEF.hpp para a respectiva porta.

### **2. Definição de Pacotes de Comunicação** (`COMM_PACKETS.hpp`)

Este arquivo é crucial para definir a estrutura dos seus dados.

* `nrf_main_packet_type_t`: 
  Enum para os tipos principais de pacotes (ex: `MAIN_PACKET_TYPE_VSSS_MESSAGE`, `MAIN_PACKET_TYPE_SSL_MESSAGE`).
* Enums de Subtipos: 
  Crie enums como `vsss_command_subtype_t` e `ssl_command_subtype_t` para detalhar os comandos específicos dentro de cada tipo principal de pacote.


```cpp

// Em COMM_PACKETS.hpp
typedef enum {
    VSSS_CMD_SUBTYPE_UNDEFINED = 0,
    VSSS_CMD_SET_MOTOR_SPEEDS,
    // adicione outros subtipos VSSS...
} vsss_command_subtype_t;

typedef enum {
    SSL_CMD_SUBTYPE_UNDEFINED = 0,
    SSL_CMD_SET_VELOCITIES,
    SSL_CMD_ACTION_CONTROL,
    // adicione outros subtipos SSL...
} ssl_command_subtype_t;
```

* **Estruturas de Payload**: Defina structs como `vsss_payload_t` e `ssl_payload_t` para conter os dados específicos de cada tipo/subtipo. Use ``` __attribute__((packed))``` para evitar padding.
* `comm_packet_header_t`: 
  Define o cabeçalho comum (tipo principal, número de sequência).
* `comm_packet_t`: A estrutura final do pacote de 32 bytes, com o cabeçalho e uma union para os diferentes payloads.

## Como Utilizar (`API`)

### **1. Inclusão de Headers**

No seu `main.cpp` ou em outros arquivos que utilizarão a biblioteca:

```cpp
#include "Comm/COMM.hpp"
```

### **2. Inicialização do Módulo NRF24** 

Agora, a inicialização é feita através da classe `Comm`:

```cpp
// Em main.cpp, dentro de /* USER CODE BEGIN 2 */
printf("Inicializando NRF24L01+...\r\n");

// Instancie a classe Comm
Comm myComm;

// Configure e inicialize a comunicação
// Os parâmetros são os mesmos da versão anterior, mas agora passados para o método Init
if (!myComm.Init(COMM_ROBOT_TYPE_SSL,
                 COMM_NODE_MODE_TRANSMITTER,
                 NRF_CHANNEL_MAIN,
                 NRF_TX_ADDRESS,
                 NRF_RX_P1_ADDRESS,
                 0 )) {
    printf("Falha ao inicializar módulo de comunicação!\r\n");
    Error_Handler();
}
printf("Módulo COMM inicializado como Transmissor SSL.\r\n");
```

### **3. Trabalhando com Pacotes (`COMM_PACKETS.hpp` e `COMM_PACKETS.cpp`)**

As funções em COMM_PACKETS.cpp ajudam a criar pacotes formatados. A criação de pacotes pode ser feita diretamente ou através dos métodos da classe `Comm`.

* `Comm_Packets_Create_VSSSMessage(comm_packet_t* packet_buffer, uint8_t seq_num, const vsss_payload_t* vsss_payload_data);`
* `Comm_Packets_Create_SSLMessage(comm_packet_t* packet_buffer, uint8_t seq_num, const ssl_payload_t* ssl_payload_data);`
* `Comm_Packets_Create_DebugText(comm_packet_t* packet_buffer, uint8_t seq_num, const char* text_payload);`

### **4. Enviando Dados (Exemplo Transmissor)**

No loop principal do transmissor (`/* USER CODE BEGIN 3 */`):

```cpp
// Em main.cpp (bloco TRANSMITTER_NODE)
ssl_payload_t ssl_command_data;

// Preencher ssl_command_data ou vsss_data
ssl_command_data.command_subtype = SSL_CMD_SET_VELOCITIES;
ssl_command_data.robot_id = 1;
ssl_command_data.vx = (int16_t)(100 + (local_packet_seq_counter % 10) * 5);
// ... preencher o resto dos campos ...

// Transmitir usando o método da instância Comm
if (myComm.Send_SSL_Message(&ssl_command_data)) {
    printf("Main: Pacote SSL (ID:%d, Vx:%d) enviado.\r\n",
           ssl_command_data.robot_id, ssl_command_data.vx);
    HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin);
} else {
    printf("Main: Falha ao enviar pacote SSL pela camada Comm.\r\n");
}
HAL_Delay(100); // Intervalo entre envios
```

### **5. Recebendo Dados (Exemplo Receptor)** 

 No loop principal do receptor (`/* USER CODE BEGIN 3 */`):

```cpp

// Em main.cpp (bloco RECEIVER_NODE)
// As funções de callback agora são registradas na instância da classe Comm

// Exemplo de registro de callbacks (deve ser feito após a inicialização da Comm)
myComm.Register_SSL_Packet_Handler(App_HandleSSLData);
myComm.Register_VSSS_Packet_Handler(App_HandleVSSSData);
myComm.Register_DebugText_Packet_Handler(App_HandleDebugText);

// Processar pacotes recebidos no loop principal
myComm.ProcessReceivedPackets();
HAL_Delay(1);
```
### STM32

<p align="center">
  <img src="./Assets/1.png">
</p>


## Exemplo no Robô `SSL`
### Transmissor:
```cpp

#include "comm/COMM.hpp"

uint8_t NRF_TX_ADDRESS[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};    // Endereço de transmissão
uint8_t NRF_RX_P1_ADDRESS[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // Endereço do Pipe 0 para ACKs no transmissor
uint8_t NRF_CHANNEL_MAIN = 76;                                 // Canal RF

ssl_payload_t ssl_command_data;
uint8_t local_packet_seq_counter = 0;

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

int main(void)
{

  printf("\r\n-- Transmissor NRF24 com Camada COMM --\r\n");
  Comm myComm; // Instancia a classe Comm
  if (!myComm.Init(COMM_ROBOT_TYPE_SSL,
                 COMM_NODE_MODE_TRANSMITTER,
                 NRF_CHANNEL_MAIN,
                 NRF_TX_ADDRESS,
                 NRF_RX_P1_ADDRESS,
                 0 )) {
      printf("Falha ao inicializar módulo de comunicação!\r\n");
      Error_Handler();
  }

  printf("Módulo COMM inicializado como Transmissor SSL.\r\n");


  while (1)
  {
	memset(&ssl_command_data, 0, sizeof(ssl_payload_t));

	ssl_command_data.command_subtype = SSL_CMD_SET_VELOCITIES; // Exemplo de subtipo
	ssl_command_data.robot_id = 1;                             // ID do Robô
	ssl_command_data.vx = (int16_t)(100 + (local_packet_seq_counter % 10) * 5); // Vx: 100, 105, ..., 145, 100...
	ssl_command_data.vy = (int16_t)(20 - (local_packet_seq_counter % 5) * 2);  // Vy: 20, 18, ..., 12, 20...
	ssl_command_data.vw = 0;
	ssl_command_data.referee_command = 0;
	ssl_command_data.kick_front = (local_packet_seq_counter % 15 == 0) ? 1 : 0; // Chuta a cada 15 pacotes
	ssl_command_data.kick_chip = 0;
	ssl_command_data.capacitor_charge = 1; // Mantém carregando
	ssl_command_data.kick_strength = 150;
	ssl_command_data.dribbler_on = (local_packet_seq_counter % 3 == 0) ? 1 : 0;
	ssl_command_data.dribbler_speed = 200;
	ssl_command_data.movement_locked = 0;
	ssl_command_data.critical_move_turbo = 0;


    if (myComm.Send_SSL_Message(&ssl_command_data)) { // Usa o método da instância
        printf("Main: Pacote SSL (ID:%d, Vx:%d) enviado.\r\n",
               ssl_command_data.robot_id, ssl_command_data.vx);
        HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin);
    } else {
        printf("Main: Falha ao enviar pacote SSL pela camada Comm.\r\n");
    }

    local_packet_seq_counter++;
  }
}
```
### Receptor:
```cpp
#include "Comm/COMM.hpp"

uint8_t NRF_RECEIVER_TX_ADDRESS_FOR_ACKS[5] = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7};
uint8_t NRF_RECEIVER_RX_P1_ADDRESS[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
uint8_t NRF_RECEIVER_RX_P2_LSB = 0xC3;
uint8_t NRF_COMMON_CHANNEL = 76;

#ifdef __GNUC__
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch) // Define a macro aqui
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}

// Funções de callback (podem ser métodos estáticos ou funções globais)
void App_HandleSSLData(const ssl_payload_t* ssl_data, uint8_t robot_id, uint8_t seq_num);
void App_HandleVSSSData(const vsss_payload_t* vsss_data, uint8_t robot_id, uint8_t seq_num);
void App_HandleDebugText(const char* text_data, uint8_t seq_num);


void App_HandleSSLData(const ssl_payload_t* ssl_data, uint8_t robot_id, uint8_t seq_num) {
    printf("CALLBACK SSL: ID=%d, Seq=%d -> Subtipo=%d, Vx=%d, Vy=%d, Vw=%d, KickF=%d, Drib=%d, Turbo=%d\r\n",
           robot_id, // robot_id é passado diretamente para o callback
           seq_num,
           ssl_data->command_subtype,
           ssl_data->vx,
           ssl_data->vy,
           ssl_data->vw,
           ssl_data->kick_front,
           ssl_data->dribbler_on,
           ssl_data->critical_move_turbo);
    // Aqui você colocaria a lógica para controlar o robô SSL com base nos dados recebidos
    // Ex: Robot_SSL_ExecuteCommand(robot_id, ssl_data);
    HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin); // Pisca LED ao processar
}

void App_HandleVSSSData(const vsss_payload_t* vsss_data, uint8_t robot_id, uint8_t seq_num) {
    printf("CALLBACK VSSS: ID=%d, Seq=%d -> Subtipo=%d, M1=%d, M2=%d, PWM?=%d\r\n",
           robot_id, // robot_id é passado diretamente para o callback
           seq_num,
           vsss_data->command_subtype,
           vsss_data->motor1_value,
           vsss_data->motor2_value,
           vsss_data->is_pwm_flag);
    // Aqui você colocaria a lógica para controlar o robô VSSS
    // Ex: Robot_VSSS_ExecuteCommand(robot_id, vsss_data);
    HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin); // Pisca LED ao processar
}

void App_HandleDebugText(const char* text_data, uint8_t seq_num) {
    printf("CALLBACK DEBUG: Seq=%d -> Texto=\'%s\'\r\n", seq_num, text_data);
    HAL_GPIO_TogglePin(LED_DEBUG_GPIO_Port, LED_DEBUG_Pin); // Pisca LED ao processar
}


int main(void)
{

  printf("\r\n-- Receptor NRF24 com Camada COMM --\r\n");

  Comm myComm; // Instancia a classe Comm
  if (!myComm.Init(COMM_ROBOT_TYPE_UNDEFINED, // Este nó é um receptor genérico ou específico
                 COMM_NODE_MODE_RECEIVER,
                 NRF_COMMON_CHANNEL,
                 NRF_RECEIVER_TX_ADDRESS_FOR_ACKS, // Endereço que o NRF usaria para enviar ACKs (Pipe0)
                 NRF_RECEIVER_RX_P1_ADDRESS,     // Endereço principal de escuta (Pipe1)
                 NRF_RECEIVER_RX_P2_LSB)) {
      printf("Falha ao inicializar módulo de comunicação como Receptor!\r\n");
      Error_Handler();
  }

  myComm.Register_SSL_Packet_Handler(App_HandleSSLData);
  myComm.Register_VSSS_Packet_Handler(App_HandleVSSSData);
  myComm.Register_DebugText_Packet_Handler(App_HandleDebugText);

  printf("Módulo COMM inicializado como Receptor. Aguardando pacotes...\r\n");

  while (1)
  {
	myComm.ProcessReceivedPackets(); // Usa o método da instância
	HAL_Delay(1);
}
}
```

## Mudanças Realizadas

As principais mudanças realizadas nesta versão da biblioteca, em comparação com a versão original em C (`CommAntiga`), são:

1.  **Portabilidade para C++**: Todos os arquivos de código-fonte (`.c`) foram convertidos para C++ (`.cpp`) e os arquivos de cabeçalho (`.h`) para `.hpp`.
2.  **Orientação a Objetos**: A funcionalidade principal da comunicação, anteriormente exposta como funções globais em C, foi encapsulada em uma classe `Comm` (definida em `COMM.hpp` e implementada em `COMM.cpp`). Isso melhora a modularidade, reusabilidade e organização do código.
3.  **Inicialização da Comunicação**: A função `Comm_Init` agora é um método da classe `Comm` (ex: `myComm.Init(...)`).
4.  **Envio de Pacotes**: As funções de envio de pacotes (ex: `Comm_Send_SSL_Message`) agora são métodos da classe `Comm` (ex: `myComm.Send_SSL_Message(...)`).
5.  **Processamento de Pacotes Recebidos**: A função `Comm_ProcessReceivedPackets` agora é um método da classe `Comm` (ex: `myComm.ProcessReceivedPackets()`).
6.  **Registro de Callbacks**: As funções para registrar callbacks (ex: `Comm_Register_SSL_Packet_Handler`) agora são métodos da classe `Comm` (ex: `myComm.Register_SSL_Packet_Handler(...)`).
7.  **Remoção de Seções Depreciadas**: As seções marcadas como `(DEPRECATED)` na versão anterior do README, que se referiam a uma API C mais antiga ou a métodos de uso menos recomendados, foram removidas ou atualizadas para refletir a nova abordagem orientada a objetos em C++.

Essas mudanças visam modernizar a base de código, aproveitando os recursos da programação orientada a objetos em C++ para uma API mais limpa e um design mais robusto.
