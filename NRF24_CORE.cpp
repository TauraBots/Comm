#include <Comm/NRF24_CORE.hpp> // Inclui o cabeçalho da nova classe NRF24_Driver
#include <Comm/NRF24_HAL.hpp> // Inclui o cabeçalho da classe NRF24_Hardware refatorada
#include <string.h>

// Construtor da classe NRF24_Driver.
// Ele recebe os parâmetros de hardware e os passa para o construtor da instância `hardware`.
NRF24_Driver::NRF24_Driver(
    SPI_HandleTypeDef* hspi,
    GPIO_TypeDef* ce_port, uint16_t ce_pin,
    GPIO_TypeDef* csn_port, uint16_t csn_pin,
    uint32_t spi_timeout_ms
) : hardware(hspi, ce_port, ce_pin, csn_port, csn_pin, spi_timeout_ms) { // Alterado: Inicializa a instância `hardware` (da classe NRF24_Hardware) com os parâmetros fornecidos.
    // Inicializações adicionais, se necessário.
}

// Método para escrever em um registrador NRF24.
// Substitui a função global `nrf24_WriteReg`.
void NRF24_Driver::writeRegister(uint8_t Reg, uint8_t Data) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t buf[2];
    buf[0] = Reg | W_REGISTER;
    buf[1] = Data;

    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    hardware.SPI_Transmit(buf, 2); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.
}

// Método para escrever múltiplos bytes em um registrador NRF24.
// Substitui a função global `nrf24_WriteRegMulti`.
void NRF24_Driver::writeRegisterMulti(uint8_t Reg, uint8_t *data, uint8_t size) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t buf[1];
    buf[0] = Reg | W_REGISTER;

    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    hardware.SPI_Transmit(buf, 1); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.SPI_Transmit(data, size); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.
}

// Método para ler um byte de um registrador NRF24.
// Substitui a função global `nrf24_ReadReg`.
uint8_t NRF24_Driver::readRegister(uint8_t Reg) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t data = 0;
    uint8_t cmd = Reg & REGISTER_MASK;

    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    hardware.SPI_Transmit(&cmd, 1); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.SPI_Receive(&data, 1); // Alterado: Chama o método SPI_Receive da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.

    return data;
}

// Método para ler múltiplos bytes de um registrador NRF24.
// Substitui a função global `nrf24_ReadReg_Multi`.
void NRF24_Driver::readRegisterMulti(uint8_t Reg, uint8_t *data, uint8_t size) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t cmd = Reg & REGISTER_MASK;

    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    hardware.SPI_Transmit(&cmd, 1); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.SPI_Receive(data, size); // Alterado: Chama o método SPI_Receive da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.
}

// Método para enviar um comando NRF (e.g., FLUSH_TX).
// Substitui a função global `nrfsendCmd`.
void NRF24_Driver::sendCommand(uint8_t cmd) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    hardware.SPI_Transmit(&cmd, 1); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.
}

// Método para resetar os registradores do NRF24 para valores padrão.
// Substitui a função global `nrf24_reset_registers`.
void NRF24_Driver::resetRegisters(void) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    hardware.CE_Disable(); // Alterado: Chama o método CE_Disable da instância `hardware`.

    // Todas as chamadas a `nrf24_WriteReg` e `nrf24_WriteRegMulti` agora são chamadas de método `this->writeRegister` e `this->writeRegisterMulti`.
    writeRegister(CONFIG, 0x08);
    writeRegister(EN_AA, 0x3F);
    writeRegister(EN_RXADDR, 0x03);
    writeRegister(SETUP_AW, 0x03);
    writeRegister(SETUP_RETR, 0x03);

    writeRegister(RF_CH, 0x02);
    writeRegister(RF_SETUP, 0x0E);   // 2mbs
    writeRegister(STATUS, (1 << RX_DR_BIT) | (1 << TX_DS_BIT) | (1 << MAX_RT_BIT));

    uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
    writeRegisterMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
    uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
    writeRegisterMulti(TX_ADDR, tx_addr_def, 5);
    uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
    writeRegisterMulti(RX_ADDR_P1, rx_addr_p1_def, 5);

    writeRegister(RX_ADDR_P2, 0xC3);
    writeRegister(RX_ADDR_P3, 0xC4);
    writeRegister(RX_ADDR_P4, 0xC5);
    writeRegister(RX_ADDR_P5, 0xC6);

    writeRegister(RX_PW_P0, 0);
    writeRegister(RX_PW_P1, 0);
    writeRegister(RX_PW_P2, 0);
    writeRegister(RX_PW_P3, 0);
    writeRegister(RX_PW_P4, 0);
    writeRegister(RX_PW_P5, 0);

    writeRegister(FIFO_STATUS, 0x11);
    writeRegister(DYNPD, 0);
    writeRegister(FEATURE, 0);

    hardware.CE_Enable(); // Alterado: Chama o método CE_Enable da instância `hardware`.
    hardware.Delay(5); // Alterado: Chama o método Delay da instância `hardware`.
}

// Método para limpar as flags de interrupção do NRF24.
// Substitui a função global `nrf24_clear_interrupts`.
void NRF24_Driver::clearInterrupts(void) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t status = readRegister(STATUS); // Alterado: Chama o método `this->readRegister`.
    status |= (1 << RX_DR_BIT) | (1 << TX_DS_BIT) | (1 << MAX_RT_BIT);
    writeRegister(STATUS, status); // Alterado: Chama o método `this->writeRegister`.
}

// Método para flushar (limpar) o FIFO de transmissão.
// Substitui a função global `nrf24_flush_tx`.
void NRF24_Driver::flushTx(void) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    sendCommand(FLUSH_TX); // Alterado: Chama o método `this->sendCommand`.
}

// Método para flushar (limpar) o FIFO de recepção.
// Substitui a função global `nrf24_flush_rx`.
void NRF24_Driver::flushRx(void) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    sendCommand(FLUSH_RX); // Alterado: Chama o método `this->sendCommand`.
}

// Método de inicialização abrangente para o NRF24.
// Substitui a função global `NRF24_Init` e adiciona parâmetros para configuração específica de endereços e canal.
void NRF24_Driver::init(uint8_t *tx_addr, uint8_t *rx_p0_addr, uint8_t *rx_p1_addr,
                        uint8_t rx_p2_lsb, uint8_t rx_p3_lsb, uint8_t rx_p4_lsb, uint8_t rx_p5_lsb,
                        uint8_t ch) { // Alterado: Parâmetros adicionados para endereços e canal.
    hardware.CE_Disable(); // Alterado: Chama o método CE_Disable da instância `hardware`.

    writeRegister(CONFIG, (1 << 3) | (1 << 2)); // Configura CRC de 2 bytes e power up
    writeRegister(EN_AA, 0x3F); // Habilita auto-ACK em todos os pipes
    writeRegister(RF_SETUP, 0x26); // Taxa de dados: 250Kbps
    writeRegister(SETUP_RETR, ((15) << 4) | NRF24_MAX_RETRANSMISSIONS); // Configura retransmissões

    uint8_t payload_size = 32; // Tamanho do payload
    writeRegister(RX_PW_P0, payload_size);
    writeRegister(RX_PW_P1, payload_size);
    writeRegister(RX_PW_P2, payload_size);
    writeRegister(RX_PW_P3, payload_size);
    writeRegister(RX_PW_P4, payload_size);
    writeRegister(RX_PW_P5, payload_size);

    writeRegister(FEATURE, 0x00); // Desabilita recursos adicionais
    writeRegister(DYNPD, 0x00);   // Desabilita payload dinâmico

    clearInterrupts(); // Alterado: Chama o método `this->clearInterrupts`.
    flushRx();         // Alterado: Chama o método `this->flushRx`.
    flushTx();         // Alterado: Chama o método `this->flushTx`.

    // Configura os endereços específicos para esta instância NRF.
    if (tx_addr) writeRegisterMulti(TX_ADDR, tx_addr, 5); // Altera o endereço TX.
    if (rx_p0_addr) writeRegisterMulti(RX_ADDR_P0, rx_p0_addr, 5); // Altera o endereço RX Pipe 0.
    if (rx_p1_addr) writeRegisterMulti(RX_ADDR_P1, rx_p1_addr, 5); // Altera o endereço RX Pipe 1.
    writeRegister(RX_ADDR_P2, rx_p2_lsb); // Altera o LSB do endereço RX Pipe 2.
    writeRegister(RX_ADDR_P3, rx_p3_lsb); // Altera o LSB do endereço RX Pipe 3.
    writeRegister(RX_ADDR_P4, rx_p4_lsb); // Altera o LSB do endereço RX Pipe 4.
    writeRegister(RX_ADDR_P5, rx_p5_lsb); // Altera o LSB do endereço RX Pipe 5.

    writeRegister(RF_CH, ch); // Configura o canal RF.

    hardware.CE_Enable(); // Alterado: Chama o método CE_Enable da instância `hardware`.
    hardware.Delay(5); // Alterado: Chama o método Delay da instância `hardware`.
}

// Método para configurar o NRF no modo Transmissão.
// Substitui a função global `NRF24_TxMode`.
void NRF24_Driver::setTxMode(uint8_t *Address, uint8_t channel) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    hardware.CE_Disable(); // Alterado: Chama o método CE_Disable da instância `hardware`.
    hardware.Delay(5); // Alterado: Chama o método Delay da instância `hardware`.

    writeRegister(RF_CH, channel); // Alterado: Chama o método `this->writeRegister`.
    writeRegisterMulti(TX_ADDR, Address, 5); // Alterado: Chama o método `this->writeRegisterMulti`.
    writeRegisterMulti(RX_ADDR_P0, Address, 5); // Alterado: Chama o método `this->writeRegisterMulti`.

    uint8_t config = readRegister(CONFIG); // Alterado: Chama o método `this->readRegister`.
    config &= ~(1 << 0); // Limpa bit PRIM_RX (modo TX)
    config |= (1 << 1);  // Power Up
    config |= (1 << 3);  // Habilita CRC
    config |= (1 << 2);  // CRC de 2 bytes
    writeRegister(CONFIG, config); // Alterado: Chama o método `this->writeRegister`.

    hardware.Delay(2); // Alterado: Chama o método Delay da instância `hardware`.

    hardware.CE_Enable(); // Alterado: Chama o método CE_Enable da instância `hardware`.
    hardware.Delay(1); // Alterado: Chama o método Delay da instância `hardware`.
}

// Método para transmitir um pacote de dados.
// Substitui a função global `NRF24_Transmit`.
uint8_t NRF24_Driver::transmit(uint8_t *data, uint8_t size) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t payload_to_send[32];
    uint8_t actual_size = size;

    if (actual_size == 0) return 0;
    if (actual_size > 32) actual_size = 32;

    memset(payload_to_send, 0, 32);
    memcpy(payload_to_send, data, actual_size);

    uint8_t cmd = W_TX_PAYLOAD;
    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    hardware.SPI_Transmit(&cmd, 1); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.SPI_Transmit(payload_to_send, 32); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.

    uint32_t start_tick = hardware.GetTick(); // Alterado: Chama o método GetTick da instância `hardware`.
    uint8_t status_reg;
    while (1) {
        status_reg = readRegister(STATUS); // Alterado: Chama o método `this->readRegister`.

        if (status_reg & ((1 << TX_DS_BIT) || (1 << MAX_RT_BIT))) { // Alterado: Usa operador OR lógico para verificar ambas as condições.
            break;
        }
        if (hardware.GetTick() - start_tick > 200) { // Alterado: Chama o método GetTick da instância `hardware`.
            flushTx(); // Alterado: Chama o método `this->flushTx`.
            return 0;
        }
    }

    writeRegister(STATUS, status_reg | (1 << TX_DS_BIT) | (1 << MAX_RT_BIT)); // Alterado: Chama o método `this->writeRegister` para limpar as flags.

    if (status_reg & (1 << TX_DS_BIT)) {
        return 1; // Transmissão bem-sucedida
    } else if (status_reg & (1 << MAX_RT_BIT)) {
        flushTx(); // Alterado: Chama o método `this->flushTx` em caso de retransmissões máximas.
        return 0;  // Falha na transmissão
    }
    return 0; // Retorno padrão para cobrir todos os casos
}

// Método para configurar o NRF no modo Recepção.
// Substitui a função global `NRF24_RxMode`.
void NRF24_Driver::setRxMode(uint8_t *AddressPipe1, uint8_t AddressPipe2LSB, uint8_t channel) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    hardware.CE_Disable(); // Alterado: Chama o método CE_Disable da instância `hardware`.
    hardware.Delay(5);     // Alterado: Chama o método Delay da instância `hardware`.

    clearInterrupts(); // Alterado: Chama o método `this->clearInterrupts`.
    flushRx();         // Alterado: Chama o método `this->flushRx`.
    flushTx();         // Alterado: Chama o método `this->flushTx`.

    writeRegister(RF_CH, channel); // Alterado: Chama o método `this->writeRegister`.
    writeRegister(EN_RXADDR, (1 << 0) | (1 << 1) | (1 << 2)); // Habilita Pipes 0, 1 e 2

    // Configura endereços dos pipes de recepção.
    writeRegisterMulti(RX_ADDR_P1, AddressPipe1, 5); // Alterado: Chama o método `this->writeRegisterMulti`.
    writeRegister(RX_ADDR_P2, AddressPipe2LSB); // Alterado: Chama o método `this->writeRegister`.

    uint8_t config = readRegister(CONFIG); // Alterado: Chama o método `this->readRegister`.
    config |= (1 << 0); // Set PRIM_RX (modo RX)
    config |= (1 << 1); // Power Up
    config |= (1 << 3); // Habilita CRC
    config |= (1 << 2); // CRC de 2 bytes
    writeRegister(CONFIG, config); // Alterado: Chama o método `this->writeRegister`.

    hardware.Delay(2); // Alterado: Chama o método Delay da instância `hardware`.

    hardware.CE_Enable(); // Alterado: Chama o método CE_Enable da instância `hardware`.
    hardware.Delay(1); // Alterado: Chama o método Delay da instância `hardware`.
}

// Método para verificar se há dados disponíveis em um pipe específico.
// Substitui a função global `isDataAvailable`.
uint8_t NRF24_Driver::isDataAvailable(uint8_t pipenum) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t status = readRegister(STATUS); // Alterado: Chama o método `this->readRegister`.
    if ((status & (1 << RX_DR_BIT)) && (((status & RX_P_NO_MASK) >> RX_P_NO_POS) == pipenum)) {
        return 1;
    }
    return 0;
}

// Método para receber dados do FIFO de recepção.
// Substitui a função global `NRF24_Receive`.
void NRF24_Driver::receive(uint8_t *data) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t cmdtosend;

    hardware.CS_Select(); // Alterado: Chama o método CS_Select da instância `hardware`.
    cmdtosend = R_RX_PAYLOAD;
    hardware.SPI_Transmit(&cmdtosend, 1); // Alterado: Chama o método SPI_Transmit da instância `hardware`.
    hardware.SPI_Receive(data, 32); // Alterado: Chama o método SPI_Receive da instância `hardware`.
    hardware.CS_UnSelect(); // Alterado: Chama o método CS_UnSelect da instância `hardware`.

    uint8_t status_reg = readRegister(STATUS); // Alterado: Chama o método `this->readRegister`.
    writeRegister(STATUS, status_reg | (1 << RX_DR_BIT)); // Alterado: Chama o método `this->writeRegister` para limpar a flag RX_DR.
}

// Método para ler todos os registradores do NRF24 (usado principalmente para depuração).
// Substitui a função global `NRF24_ReadAll`.
void NRF24_Driver::readAllRegisters(uint8_t *data) { // Alterado: Tornou-se um método da classe NRF24_Driver.
    uint8_t j = 0;

    for (int i = 0; i <= 0x09; i++) {
        *(data + j++) = readRegister(i); // Alterado: Chama o método `this->readRegister`.
    }

    readRegisterMulti(RX_ADDR_P0, (data + j), 5); // Alterado: Chama o método `this->readRegisterMulti`.
    j += 5;

    readRegisterMulti(RX_ADDR_P1, (data + j), 5); // Alterado: Chama o método `this->readRegisterMulti`.
    j += 5;

    *(data + j++) = readRegister(RX_ADDR_P2); // Alterado: Chama o método `this->readRegister`.
    *(data + j++) = readRegister(RX_ADDR_P3); // Alterado: Chama o método `this->readRegister`.
    *(data + j++) = readRegister(RX_ADDR_P4); // Alterado: Chama o método `this->readRegister`.
    *(data + j++) = readRegister(RX_ADDR_P5); // Alterado: Chama o método `this->readRegister`.

    readRegisterMulti(TX_ADDR, (data + j), 5); // Alterado: Chama o método `this->readRegisterMulti`.
    j += 5;

    for (int i = 0x11; i <= 0x16; i++) {
        *(data + j++) = readRegister(i); // Alterado: Chama o método `this->readRegister`.
    }

    *(data + j++) = readRegister(FIFO_STATUS); // Alterado: Chama o método `this->readRegister`.
    *(data + j++) = readRegister(DYNPD); // Alterado: Chama o método `this->readRegister`.
    *(data + j++) = readRegister(FEATURE); // Alterado: Chama o método `this->readRegister`.
}