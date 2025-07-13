#include "COMM.hpp" // Inclui o novo cabeçalho da classe CommManager
#include "NRF24_CORE.hpp" // Inclui a nova classe NRF24_Driver (anteriormente NRF24_CORE)
#include "COMM_PACKETS.hpp" // Permanece inalterado, define as estruturas de pacotes

// Definições de endereços para NRF24L01+
// Estes endereços são exemplos e devem ser únicos para cada robô/torre
// e para os pipes de comunicação.
// Alterado: Movido de variáveis estáticas globais para constantes globais visíveis neste arquivo.
const uint8_t ADDR_BASE_LISTEN[5] = {0xB1, 0xB2, 0xB3, 0xB4, 0xB5}; // Endereço de escuta comum para a torre
const uint8_t ADDR_ROBOT_LISTEN[MAX_ROBOTS][5] = { // Endereços de escuta específicos para cada robô (RX do robô, TX da torre)
    {0xA1, 0xA2, 0xA3, 0xA4, 0xA5}, // Robô 1
    {0xC1, 0xC2, 0xC3, 0xC4, 0xC5}, // Robô 2
    {0xD1, 0xD2, 0xD3, 0xD4, 0xD5}, // Robô 3
    {0xE1, 0xE2, 0xE3, 0xE4, 0xE5}, // Robô 4
    {0xF1, 0xF2, 0xF3, 0xF4, 0xF5}  // Robô 5
};

// --- Início da Implementação da Classe CommManager ---
// A classe CommManager encapsula toda a lógica de comunicação de alto nível.
// Ela substitui as funções C globais e variáveis estáticas da versão anterior,
// fornecendo uma interface orientada a objetos para gerenciar múltiplos NRFs.

// Construtor: Inicializa os membros da classe.
// Alterado: Agora é um construtor de classe, inicializando ponteiros NRF para nulos e callbacks.
CommManager::CommManager() :
    current_robot_type(COMM_ROBOT_TYPE_NONE), // Inicializa o tipo de robô como 'nenhum'
    current_node_mode(COMM_NODE_MODE_NONE),   // Inicializa o modo do nó como 'nenhum'
    packet_seq_counter(0),                     // Contador de sequência de pacotes
    ssl_handler(nullptr),                      // Handlers de callback inicializados como nulos
    vsss_handler(nullptr),
    debug_text_handler(nullptr),
    rx_nrf(nullptr),                           // Ponteiros para NRF_Driver inicializados como nulos
    tx_nrf(nullptr),
    telemetry_rx_nrf(nullptr)
{
    // Inicializa o array de ponteiros command_tx_nrf para nullptr
    for (int i = 0; i < MAX_ROBOTS; ++i) {
        command_tx_nrf[i] = nullptr;
    }
}

// Destrutor: Libera a memória alocada dinamicamente para as instâncias de NRF24_Driver.
// Adicionado: Essencial para gerenciar a memória quando se usa 'new'.
CommManager::~CommManager() {
    if (rx_nrf) delete rx_nrf;
    if (tx_nrf) delete tx_nrf;
    if (telemetry_rx_nrf) delete telemetry_rx_nrf;
    for (int i = 0; i < MAX_ROBOTS; ++i) {
        if (command_tx_nrf[i]) delete command_tx_nrf[i];
    }
}

// O método Init configura a CommManager para operar como Torre ou Robô,
// inicializando os módulos NRF24_Driver com as configurações de hardware e endereços apropriados.
// Alterado: Substitui a função global Comm_Init. Recebe configurações de hardware para todos os NRFs.
bool CommManager::Init(
    comm_robot_type_t robot_type,
    comm_node_mode_t node_mode,
    const NRF24_Hardware_Config& nrf_config_main_rx, // Config para o NRF RX principal (Robô: RX, Torre: Telemetria RX)
    const NRF24_Hardware_Config& nrf_config_main_tx, // Config para o NRF TX principal (Robô: TX)
    const NRF24_Hardware_Config command_tx_configs[MAX_ROBOTS], // Array de configs para NRFs TX de comando (Torre)
    uint8_t robot_id // ID do robô (1-5) se for um robô, 0 se for a torre
) {
    current_robot_type = robot_type;
    current_node_mode = node_mode;

    // Dependendo do modo (Torre/Robô), instanciar e configurar os NRFs apropriados
    if (current_node_mode == COMM_NODE_MODE_RECEIVER) { // Modo Robô
        if (robot_id == 0 || robot_id > MAX_ROBOTS) return false; // ID do robô inválido

        // NRF de Recepção (RX) de Comandos: escuta no seu próprio endereço de robô
        // Instancia NRF24_Hardware para o RX do robô com base na config fornecida
        NRF24_Hardware rx_hw(
            nrf_config_main_rx.hspi,
            nrf_config_main_rx.ce_port, nrf_config_main_rx.ce_pin,
            nrf_config_main_rx.csn_port, nrf_config_main_rx.csn_pin,
            nrf_config_main_rx.spi_timeout_ms
        );
        rx_nrf = new NRF24_Driver(rx_hw); // Cria uma nova instância do driver NRF para RX
        // Inicializa em modo RX com o endereço do robô (ADDR_ROBOT_LISTEN)
        // O segundo parâmetro é o endereço de TX do pipe 0, usado para ACKs.
        // O terceiro parâmetro é o endereço de TX padrão para quando o NRF está em modo RX
        // e responde com um pacote (não utilizado para RX).
        if (!rx_nrf->Init(NRF_RX_MODE, ADDR_ROBOT_LISTEN[robot_id - 1], ADDR_BASE_LISTEN, NRF24_RF_CHANNEL)) return false;
        if (!rx_nrf->RxMode()) return false; // Coloca o NRF em modo de escuta

        // NRF de Transmissão (TX) de Telemetria: transmite para o endereço base da torre
        // Instancia NRF24_Hardware para o TX do robô com base na config fornecida
        NRF24_Hardware tx_hw(
            nrf_config_main_tx.hspi,
            nrf_config_main_tx.ce_port, nrf_config_main_tx.ce_pin,
            nrf_config_main_tx.csn_port, nrf_config_main_tx.csn_pin,
            nrf_config_main_tx.spi_timeout_ms
        );
        tx_nrf = new NRF24_Driver(tx_hw); // Cria uma nova instância do driver NRF para TX
        // Inicializa em modo TX com o endereço base da torre como endereço de TX (Pipe 0)
        // O segundo parâmetro é o endereço de escuta do próprio NRF em modo TX (não usado).
        // O terceiro parâmetro é o endereço de TX para o Pipe 0.
        if (!tx_nrf->Init(NRF_TX_MODE, ADDR_BASE_LISTEN, ADDR_BASE_LISTEN, NRF24_RF_CHANNEL)) return false;
        if (!tx_nrf->TxMode()) return false; // Coloca o NRF em modo de transmissão

    } else if (current_node_mode == COMM_NODE_MODE_TRANSMITTER) { // Modo Torre
        // NRF de Recepção (RX) de Telemetria: escuta em múltiplos pipes para todos os robôs
        // Instancia NRF24_Hardware para o RX de telemetria da torre
        NRF24_Hardware telemetry_rx_hw(
            nrf_config_main_rx.hspi,
            nrf_config_main_rx.ce_port, nrf_config_main_rx.ce_pin,
            nrf_config_main_rx.csn_port, nrf_config_main_rx.csn_pin,
            nrf_config_main_rx.spi_timeout_ms
        );
        telemetry_rx_nrf = new NRF24_Driver(telemetry_rx_hw); // Instância para o NRF de RX de telemetria

        // Inicializa o NRF de telemetria para escutar no endereço base da torre (Pipe 0).
        // Este é o endereço que os robôs irão transmitir.
        if (!telemetry_rx_nrf->Init(NRF_RX_MODE, ADDR_BASE_LISTEN, ADDR_BASE_LISTEN, NRF24_RF_CHANNEL)) return false;
        
        // Configura pipes adicionais para escutar a telemetria de cada robô.
        // A torre deve ouvir no endereço de transmissão de cada robô para receber ACKs e dados diretos.
        // Pipe 1 ao Pipe 5 para os 5 robôs.
        // Alterado: Adiciona a configuração dos pipes de leitura para cada robô.
        for (int i = 0; i < MAX_ROBOTS; ++i) {
            if (!telemetry_rx_nrf->OpenReadingPipe(i + 1, ADDR_ROBOT_LISTEN[i])) return false; // Pipe i+1 escuta no endereço do robô i
        }
        if (!telemetry_rx_nrf->RxMode()) return false; // Coloca o NRF em modo de escuta


        // NRFs de Transmissão (TX) de Comandos: um NRF por robô, transmite para o endereço do robô específico
        // Alterado: Cria e inicializa um array de NRF24_Driver para cada robô.
        for (int i = 0; i < MAX_ROBOTS; ++i) {
            NRF24_Hardware command_tx_hw(
                command_tx_configs[i].hspi,
                command_tx_configs[i].ce_port, command_tx_configs[i].ce_pin,
                command_tx_configs[i].csn_port, command_tx_configs[i].csn_pin,
                command_tx_configs[i].spi_timeout_ms
            );
            command_tx_nrf[i] = new NRF24_Driver(command_tx_hw); // Cria uma instância para cada NRF de TX

            // Inicializa cada NRF de comando para transmitir para o endereço de escuta do robô correspondente
            if (!command_tx_nrf[i]->Init(NRF_TX_MODE, ADDR_ROBOT_LISTEN[i], ADDR_ROBOT_LISTEN[i], NRF24_RF_CHANNEL)) return false;
            if (!command_tx_nrf[i]->TxMode()) return false; // Coloca em modo de transmissão
        }
    } else {
        return false; // Modo desconhecido
    }
    
    // O canal de RF é configurado individualmente para cada NRF_Driver durante sua inicialização,
    // garantindo consistência através de todos os módulos NRF.

    return true;
}

// O método Loop() é responsável por processar os pacotes de comunicação.
// Deve ser chamado repetidamente no loop principal da aplicação.
// Alterado: Substitui a função global Comm_Loop. Gerencia a leitura de pacotes
// de múltiplos NRFs dependendo do modo do nó.
void CommManager::Loop() {
    comm_packet_t received_packet; // Buffer para o pacote recebido
    uint8_t pipe_num; // Número do pipe de onde o pacote foi recebido

    // Lógica para Robô (Receiver)
    if (current_node_mode == COMM_NODE_MODE_RECEIVER && rx_nrf != nullptr) {
        if (rx_nrf->IsPacketAvailable(&pipe_num)) { // Verifica se há pacote disponível no NRF de RX do robô
            if (rx_nrf->ReadPacket((uint8_t*)&received_packet, sizeof(comm_packet_t))) { // Lê o pacote
                // Dispara o callback apropriado com base no tipo de pacote
                if (received_packet.type == NRF_MAIN_PACKET_TYPE_SSL_COMMAND && ssl_handler) {
                    ssl_handler(current_robot_type, received_packet.payload.ssl_payload);
                } else if (received_packet.type == NRF_MAIN_PACKET_TYPE_VSSS_COMMAND && vsss_handler) {
                    vsss_handler(current_robot_type, received_packet.payload.vsss_payload);
                } else if (received_packet.type == NRF_MAIN_PACKET_TYPE_DEBUG_TEXT && debug_text_handler) {
                    debug_text_handler(current_robot_type, received_packet.payload.debug_text_payload.text);
                }
            }
        }
    }
    // Lógica para Torre (Transmitter)
    else if (current_node_mode == COMM_NODE_MODE_TRANSMITTER && telemetry_rx_nrf != nullptr) {
        if (telemetry_rx_nrf->IsPacketAvailable(&pipe_num)) { // Verifica se há telemetria disponível no NRF de RX da torre
            if (telemetry_rx_nrf->ReadPacket((uint8_t*)&received_packet, sizeof(comm_packet_t))) { // Lê o pacote
                // A telemetria deve conter o ID do robô no payload para identificação.
                // O pipe_num também pode indicar a origem (pipe 1 para robô 1, etc.).
                // Alterado: O callback para telemetria na torre precisa ser capaz de identificar o robô de origem.
                // Aqui, o ssl_handler (que era para comandos) está sendo reutilizado, mas para telemetria SSL.
                // Pode ser necessário criar um novo tipo de callback para telemetria que inclua o ID do robô.
                if (received_packet.type == NRF_MAIN_PACKET_TYPE_SSL_TELEMETRY && ssl_handler) {
                    // Nota: 'current_robot_type' aqui se refere ao tipo da Torre (ex: COMM_ROBOT_TYPE_SSL),
                    // não ao tipo do robô que enviou. O ID do robô de origem pode ser extraído de `received_packet.payload.ssl_telemetry_payload.robot_id`.
                    ssl_handler(current_robot_type, received_packet.payload.ssl_telemetry_payload);
                }
                 else if (received_packet.type == NRF_MAIN_PACKET_TYPE_DEBUG_TEXT && debug_text_handler) {
                    debug_text_handler(current_robot_type, received_packet.payload.debug_text_payload.text);
                }
            }
        }
    }
}

// Métodos para envio de mensagens (comandos da torre, telemetria do robô)
// Alterado: Substituem as funções globais Comm_Send_*. Agora são métodos da classe.
// Incluem lógica para selecionar o NRF_Driver correto com base no modo e no target_robot_id.

bool CommManager::Send_VSSS_Message(uint8_t target_robot_id, const vsss_payload_t* vsss_payload_data) {
    // Se for robô, usa seu NRF de TX (target_robot_id é ignorado aqui, pois robô só envia sua própria telemetria ou debug)
    if (current_node_mode == COMM_NODE_MODE_RECEIVER && tx_nrf != nullptr) {
        Comm_Packets_Create_VSSSMessage(packet_seq_counter++, vsss_payload_data, &comm_packet_buffer);
        return tx_nrf->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    // Se for torre, usa o NRF TX específico do robô alvo.
    else if (current_node_mode == COMM_NODE_MODE_TRANSMITTER && target_robot_id > 0 && target_robot_id <= MAX_ROBOTS && command_tx_nrf[target_robot_id - 1] != nullptr) {
        Comm_Packets_Create_VSSSMessage(packet_seq_counter++, vsss_payload_data, &comm_packet_buffer);
        return command_tx_nrf[target_robot_id - 1]->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    return false;
}

bool CommManager::Send_SSL_Message(uint8_t target_robot_id, const ssl_payload_t* ssl_payload_data) {
    // Se for robô, usa seu NRF de TX
    if (current_node_mode == COMM_NODE_MODE_RECEIVER && tx_nrf != nullptr) {
        Comm_Packets_Create_SSLMessage(packet_seq_counter++, ssl_payload_data, &comm_packet_buffer);
        return tx_nrf->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    // Se for torre, usa o NRF TX específico do robô alvo.
    else if (current_node_mode == COMM_NODE_MODE_TRANSMITTER && target_robot_id > 0 && target_robot_id <= MAX_ROBOTS && command_tx_nrf[target_robot_id - 1] != nullptr) {
        Comm_Packets_Create_SSLMessage(packet_seq_counter++, ssl_payload_data, &comm_packet_buffer);
        return command_tx_nrf[target_robot_id - 1]->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    return false;
}

// Método de envio de telemetria SSL (geralmente usado pelo Robô)
// Alterado: Novo método, específico para telemetria SSL, transmitido pelo NRF TX do robô.
bool CommManager::Send_SSL_Telemetry(uint8_t target_robot_id, const ssl_telemetry_payload_t* telemetry_payload_data) {
    // A telemetria é sempre enviada pelo robô para a torre.
    // O target_robot_id neste contexto é o ID do robô que está enviando a telemetria,
    // garantindo que a telemetria contém essa informação.
    // O NRF_TX_MODE do robô já está configurado para transmitir para o endereço da torre.
    if (current_node_mode == COMM_NODE_MODE_RECEIVER && tx_nrf != nullptr) {
        Comm_Packets_Create_SSLTelemetryMessage(packet_seq_counter++, telemetry_payload_data, &comm_packet_buffer);
        return tx_nrf->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    return false; // A torre não envia telemetria SSL (ela recebe)
}

bool CommManager::Send_DebugText_Message(uint8_t target_robot_id, const char* text_payload) {
    // Se for robô, usa seu NRF de TX
    if (current_node_mode == COMM_NODE_MODE_RECEIVER && tx_nrf != nullptr) {
        Comm_Packets_Create_DebugTextMessage(packet_seq_counter++, text_payload, &comm_packet_buffer);
        return tx_nrf->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    // Se for torre, usa o NRF TX específico do robô alvo.
    else if (current_node_mode == COMM_NODE_MODE_TRANSMITTER && target_robot_id > 0 && target_robot_id <= MAX_ROBOTS && command_tx_nrf[target_robot_id - 1] != nullptr) {
        Comm_Packets_Create_DebugTextMessage(packet_seq_counter++, text_payload, &comm_packet_buffer);
        return command_tx_nrf[target_robot_id - 1]->Transmit((uint8_t*)&comm_packet_buffer, sizeof(comm_packet_t));
    }
    return false;
}

// Métodos para registrar os callbacks de tratamento de pacotes.
// Alterado: Substituem as funções globais Set*Callback. Agora são métodos da classe.
void CommManager::SetSSLCallback(comm_ssl_packet_handler_t handler) {
    ssl_handler = handler; // Atribui o handler fornecido ao membro da classe
}

void CommManager::SetVSSSCallback(comm_vsss_packet_handler_t handler) {
    vsss_handler = handler; // Atribui o handler fornecido ao membro da classe
}

void CommManager::SetDebugTextCallback(comm_debug_text_packet_handler_t handler) {
    debug_text_handler = handler; // Atribui o handler fornecido ao membro da classe
}