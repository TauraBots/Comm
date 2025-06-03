/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Comm/COMM.h"
#include "Comm/COMM_PACKETS.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;    // Conforme referenciado em NRF24_DEF.h
UART_HandleTypeDef huart2;  // Para printf

// Variáveis globais para cálculo de perda de pacotes
uint32_t g_receivedPacketCount = 0;
uint32_t g_lostPacketCount = 0;
uint8_t  g_lastSequenceNumber = 0;
uint8_t  g_firstPacketFlag = 1;

uint32_t g_statsDisplayCounter = 0;
const uint32_t STATS_DISPLAY_INTERVAL = 50;

// --- Endereços e Canal NRF24L01+ ---
uint8_t NRF_RECEIVER_LISTEN_ADDRESS[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // Endereço que o TX envia
uint8_t NRF_RECEIVER_TX_ADDRESS[5]   = {0xD7, 0xD7, 0xD7, 0xD7, 0xD7}; // Endereço se o RX precisar transmitir
uint8_t NRF_CHANNEL = 76; // MESMO CANAL DO TRANSMISSOR

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);

void App_VSSS_Packet_Handler(const vsss_payload_t* vsss_data, uint8_t robot_id_from_payload, uint8_t seq_num);
void App_SSL_Packet_Handler(const ssl_payload_t* ssl_data, uint8_t robot_id_from_payload, uint8_t seq_num);
void App_DebugText_Packet_Handler(const char* text_data, uint8_t seq_num);

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
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();

  printf("RECEPTOR STM32 NRF24L01+ Inicializado\r\n");

  if (Comm_Init(COMM_ROBOT_TYPE_UNDEFINED,
                 COMM_NODE_MODE_RECEIVER,
                 NRF_CHANNEL,
                 NRF_RECEIVER_TX_ADDRESS,
                 NRF_RECEIVER_LISTEN_ADDRESS,
                 0xC3) == true)
  {
    printf("Biblioteca COMM inicializada como RECEPTOR.\r\n");
    printf("Escutando no Canal: %d em Endereco: %02X:%02X:%02X:%02X:%02X\r\n",
           NRF_CHANNEL, NRF_RECEIVER_LISTEN_ADDRESS[0], NRF_RECEIVER_LISTEN_ADDRESS[1],
           NRF_RECEIVER_LISTEN_ADDRESS[2], NRF_RECEIVER_LISTEN_ADDRESS[3], NRF_RECEIVER_LISTEN_ADDRESS[4]);
  }
  else
  {
    printf("ERRO: Falha ao inicializar a biblioteca COMM!\r\n");
    Error_Handler();
  }

  Comm_Register_VSSS_Packet_Handler(App_VSSS_Packet_Handler);
  Comm_Register_SSL_Packet_Handler(App_SSL_Packet_Handler);
  Comm_Register_DebugText_Packet_Handler(App_DebugText_Packet_Handler);

  printf("Handlers registrados. Aguardando pacotes...\r\n");

  while (1)
  {
    Comm_Process_Received_Messages();
    HAL_Delay(1); // Pequeno delay para não sobrecarregar a CPU
  }
}

void ProcessPacketLoss(uint8_t current_seq_num) {
    // Esta é a primeira chamada ao handler após o início ou reset das estatísticas
    if (g_firstPacketFlag) {
        g_lastSequenceNumber = current_seq_num;
        g_firstPacketFlag = 0;
        g_receivedPacketCount++; // Conta o primeiro pacote
        printf("    Info: Primeiro pacote. Base de sequencia: %u\r\n", g_lastSequenceNumber);
    } else {
        // Se o número de sequência for diferente do último, processa
        if (current_seq_num != g_lastSequenceNumber) {
            g_receivedPacketCount++; // Conta como um novo pacote recebido
            uint8_t diff;
            if (current_seq_num > g_lastSequenceNumber) {
                diff = current_seq_num - g_lastSequenceNumber;
            } else { // Wrap-around
                diff = (255 - g_lastSequenceNumber) + current_seq_num + 1; // (256 + current) - last
            }

            if (diff > 1) {
                uint8_t lost_now = diff - 1;
                g_lostPacketCount += lost_now;
                printf("    ALERTA: %u pacote(s) perdido(s)! (Atual: %u, Ultimo: %u)\r\n",
                       lost_now, current_seq_num, g_lastSequenceNumber);
            }
            g_lastSequenceNumber = current_seq_num;
        } else {
             // Mesmo número de sequência recebido. Não conta como novo para perda,
             // nem atualiza lastSequenceNumber. g_receivedPacketCount NÃO é incrementado aqui.
             printf("    Info: Mesmo numero de sequencia (%u) recebido novamente.\r\n", current_seq_num);
        }
    }

    // Atualiza contador para exibir estatísticas
    // (somente se um novo pacote válido foi processado para contagem de perda)
    if (g_receivedPacketCount > 0 && (g_statsDisplayCounter % STATS_DISPLAY_INTERVAL == 0 || g_firstPacketFlag == 0 && current_seq_num != g_lastSequenceNumber) ) {
       g_statsDisplayCounter++; // Incrementa apenas quando for exibir ou quando um pacote novo chegar
    }
    
    if (g_statsDisplayCounter >= STATS_DISPLAY_INTERVAL && g_receivedPacketCount > 0) {
        float lossRate = 0.0f;
        uint32_t expected_packets_since_first = g_receivedPacketCount + g_lostPacketCount;

        if (expected_packets_since_first > 0) { // Garante que receivedCount já é >0 por causa do if anterior
             lossRate = ((float)g_lostPacketCount / (float)expected_packets_since_first) * 100.0f;
        }
        printf("--------------------------------STATS GLOBAIS-------------------------------\r\n");
        printf("Pacotes Unicos Processados: %lu | Pacotes Perdidos Estimados: %lu\r\n",
               g_receivedPacketCount, g_lostPacketCount);
        printf("Taxa de Perda Estimada: %.2f%%\r\n", lossRate);
        printf("Ultimo Seq. Unico Processado: %u\r\n", g_lastSequenceNumber);
        printf("--------------------------------------------------------------------------\r\n");
        g_statsDisplayCounter = 0; // Reseta o contador de intervalo de exibição
    }
}

void App_VSSS_Packet_Handler(const vsss_payload_t* vsss_data, uint8_t robot_id_from_payload, uint8_t seq_num) {
    printf("-- Pacote VSSS Recebido (Seq: %u, ID Robo: %u) --\r\n", seq_num, robot_id_from_payload);
    printf("    Subtipo: %u, M1: %u, M2: %u\r\n",
           vsss_data->command_subtype, vsss_data->motor1_value, vsss_data->motor2_value);
    ProcessPacketLoss(seq_num);
}

void App_SSL_Packet_Handler(const ssl_payload_t* ssl_data, uint8_t robot_id_from_payload, uint8_t seq_num) {
    printf("-- Pacote SSL Recebido (Seq: %u, ID Robo: %u) --\r\n", seq_num, robot_id_from_payload);
    printf("    Subtipo: %u, VelX: %d, VelY: %d, VelAng: %d\r\n",
           ssl_data->command_subtype, ssl_data->vel_x, ssl_data->vel_y, ssl_data->vel_angular);
    ProcessPacketLoss(seq_num);
}

void App_DebugText_Packet_Handler(const char* text_data, uint8_t seq_num) {
    printf("-- Pacote Debug Text Recebido (Seq: %u) --\r\n", seq_num);
    printf("    Texto: \"%s\"\r\n", text_data);
    ProcessPacketLoss(seq_num);
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0}; // Necessário para HSE_VALUE se usado por USB etc.

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  // Tenta usar HSE se disponível (mais estável)
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON; // Se sua placa tiver um cristal externo (ex: 8MHz ou 25MHz)
  // RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS; // Se usar um clock externo em vez de cristal
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  // Para STM32F411CE (Black Pill comum com cristal de 25MHz) para 100MHz SYSCLK:
  // PLLM = 25 (para HSE=25MHz -> VCO input = 1MHz) ou PLLM = 8 (para HSE=8MHz -> VCO input = 1MHz)
  // Assumindo HSE = 25 MHz (Comum em BlackPill F411)
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 200; // VCO output = 1MHz * 200 = 200MHz
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // SYSCLK = VCO / 2 = 100MHz
  RCC_OscInitStruct.PLL.PLLQ = 4; // Para USB (OTG_FS clock = 200MHz / 4 = 50MHz, precisa de 48MHz, ajuste PLLN ou PLLQ)
                                   // Para 48MHz USB: PLLN=192, PLLQ=4 -> SYSCLK=96MHz, USB=48MHz
                                   // Ou PLLN=200, PLLQ= (200/48) = 4.16 - não ideal.
                                   // Se USB não for crítico agora, PLLN=200, PLLQ=4 é um começo.
  // Se usar HSI (16MHz interno)
  // RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  // RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  // RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  // RCC_OscInitStruct.PLL.PLLM = 8; // VCO input = 16MHz / 8 = 2MHz
  // RCC_OscInitStruct.PLL.PLLN = 100; // VCO output = 2MHz * 100 = 200MHz
  // RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // SYSCLK = 100MHz

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1; // HCLK = SYSCLK (100MHz)
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  // APB1 PCLK1 = HCLK/2 (50MHz)
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  // APB2 PCLK2 = HCLK/1 (100MHz)

  // Latência do Flash para 100MHz (STM32F411: 3 wait states se VOS=1, entre 90-100MHz)
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) { Error_Handler(); }

  // Para USB, se HSE for usado e PLLQ configurado:
  // PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  // PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  // if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) { Error_Handler(); }
}

static void MX_SPI1_Init(void) {
  hspi1.Instance = SPI1; // Conforme NRF24_DEF.h
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  // Se PCLK2 (clock do SPI1) = 100MHz:
  // Prescaler 16 => 100/16 = 6.25 MHz (Seguro para NRF24L01+, max 10MHz)
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK) { Error_Handler(); }
}

static void MX_USART2_UART_Init(void) { // Ou a UART que você usa para printf
  huart2.Instance = USART2; // Exemplo para Nucleo boards (conectado ao ST-Link)
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX; // TX é suficiente para printf
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK) { Error_Handler(); }
}

static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Habilitar clocks para as portas GPIO usadas por NRF24_CE_PORT e NRF24_CSN_PORT
  // (definidas em NRF24_DEF.h) e quaisquer outros GPIOs (LEDs).
  // Exemplo: Se CE e CSN estão em GPIOB
  __HAL_RCC_GPIOB_CLK_ENABLE();
  // Se você tiver um LED em PA5 (comum em Nucleo)
  // __HAL_RCC_GPIOA_CLK_ENABLE();

  // Pinos CE e CSN do NRF24L01+
  HAL_GPIO_WritePin(NRF24_CE_PORT, NRF24_CE_PIN, GPIO_PIN_RESET); // CE Baixo (inativo)
  HAL_GPIO_WritePin(NRF24_CSN_PORT, NRF24_CSN_PIN, GPIO_PIN_SET); // CSN Alto (inativo)

  GPIO_InitStruct.Pin = NRF24_CE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // Alta velocidade para CE
  HAL_GPIO_Init(NRF24_CE_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = NRF24_CSN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP; // CSN é ativo baixo, pull-up é seguro
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // Alta velocidade para CSN
  HAL_GPIO_Init(NRF24_CSN_PORT, &GPIO_InitStruct);

  // Exemplo de configuração de LED (se PA5 for um LED)
  /*
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  */
}

void Error_Handler(void) {
  __disable_irq();
  printf("ERRO FATAL - Sistema Paralisado.\r\n");
  // Piscar um LED aqui pode ser útil para indicar o erro sem UART
  while (1) { /* Loop infinito */ }
}