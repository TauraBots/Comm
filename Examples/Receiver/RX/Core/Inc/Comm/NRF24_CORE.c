#include <Comm/NRF24_CORE.h>
#include <Comm/NRF24_HAL.h>
#include <string.h>

void nrf24_WriteReg(uint8_t Reg, uint8_t Data) {
    uint8_t buf[2];
    buf[0] = Reg | W_REGISTER;
    buf[1] = Data;

    NRF24_HAL_CS_Select();
    NRF24_HAL_SPI_Transmit(buf, 2, 100);
    NRF24_HAL_CS_UnSelect();
}

void nrf24_WriteRegMulti(uint8_t Reg, uint8_t *data, uint8_t size) {
    uint8_t buf[1];
    buf[0] = Reg | W_REGISTER;

    NRF24_HAL_CS_Select();
    NRF24_HAL_SPI_Transmit(buf, 1, 100);
    NRF24_HAL_SPI_Transmit(data, size, 1000);
    NRF24_HAL_CS_UnSelect();
}

uint8_t nrf24_ReadReg(uint8_t Reg) {
    uint8_t data = 0;
    uint8_t cmd = Reg & REGISTER_MASK;

    NRF24_HAL_CS_Select();
    NRF24_HAL_SPI_Transmit(&cmd, 1, 100);
    NRF24_HAL_SPI_Receive(&data, 1, 100);
    NRF24_HAL_CS_UnSelect();

    return data;
}

void nrf24_ReadReg_Multi(uint8_t Reg, uint8_t *data, uint8_t size) {
    uint8_t cmd = Reg & REGISTER_MASK;

    NRF24_HAL_CS_Select();
    NRF24_HAL_SPI_Transmit(&cmd, 1, 100);
    NRF24_HAL_SPI_Receive(data, size, 1000);
    NRF24_HAL_CS_UnSelect();
}

void nrfsendCmd(uint8_t cmd) {
    NRF24_HAL_CS_Select();
    NRF24_HAL_SPI_Transmit(&cmd, 1, 100);
    NRF24_HAL_CS_UnSelect();
}


void nrf24_reset_registers(void) {
    NRF24_HAL_CE_Disable();

    nrf24_WriteReg(CONFIG, 0x08);
    nrf24_WriteReg(EN_AA, 0x3F);
    nrf24_WriteReg(EN_RXADDR, 0x03);
    nrf24_WriteReg(SETUP_AW, 0x03);
    nrf24_WriteReg(SETUP_RETR, 0x03);


    nrf24_WriteReg(RF_CH, 0x02);
    nrf24_WriteReg(RF_SETUP, 0x0E);   // 2mbs
    nrf24_WriteReg(STATUS, (1 << RX_DR_BIT) | (1 << TX_DS_BIT) | (1 << MAX_RT_BIT));


    uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
    nrf24_WriteRegMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
    uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
    nrf24_WriteRegMulti(TX_ADDR, tx_addr_def, 5);
    uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
    nrf24_WriteRegMulti(RX_ADDR_P1, rx_addr_p1_def, 5);

    nrf24_WriteReg(RX_ADDR_P2, 0xC3);
    nrf24_WriteReg(RX_ADDR_P3, 0xC4);
    nrf24_WriteReg(RX_ADDR_P4, 0xC5);
    nrf24_WriteReg(RX_ADDR_P5, 0xC6);


    nrf24_WriteReg(RX_PW_P0, 0);
    nrf24_WriteReg(RX_PW_P1, 0);
    nrf24_WriteReg(RX_PW_P2, 0);
    nrf24_WriteReg(RX_PW_P3, 0);
    nrf24_WriteReg(RX_PW_P4, 0);
    nrf24_WriteReg(RX_PW_P5, 0);

    nrf24_WriteReg(FIFO_STATUS, 0x11);
    nrf24_WriteReg(DYNPD, 0);
    nrf24_WriteReg(FEATURE, 0);

    NRF24_HAL_CE_Enable();
    NRF24_HAL_Delay(5);
}


void nrf24_clear_interrupts(void) {
    uint8_t status = nrf24_ReadReg(STATUS);
    status |= (1 << RX_DR_BIT) | (1 << TX_DS_BIT) | (1 << MAX_RT_BIT);
    nrf24_WriteReg(STATUS, status);
}

void nrf24_flush_tx(void) {
    nrfsendCmd(FLUSH_TX);
}

void nrf24_flush_rx(void) {
    nrfsendCmd(FLUSH_RX);
}


void NRF24_Init(void) {
    NRF24_HAL_CE_Disable();


    nrf24_WriteReg(CONFIG, (1 << 3) | (1 << 2));
    nrf24_WriteReg(EN_AA, 0x3F);

    // Taxa de dados: 250Kbps (RF_SETUP=0x26)
    nrf24_WriteReg(RF_SETUP, 0x26);
    nrf24_WriteReg(SETUP_RETR, ((15) << 4) | NRF24_MAX_RETRANSMISSIONS);

    uint8_t payload_size = 32;
    nrf24_WriteReg(RX_PW_P0, payload_size);
    nrf24_WriteReg(RX_PW_P1, payload_size);
    nrf24_WriteReg(RX_PW_P2, payload_size);
    nrf24_WriteReg(RX_PW_P3, payload_size);
    nrf24_WriteReg(RX_PW_P4, payload_size);
    nrf24_WriteReg(RX_PW_P5, payload_size);


    nrf24_WriteReg(FEATURE, 0x00);
    nrf24_WriteReg(DYNPD, 0x00);

    nrf24_clear_interrupts();
    nrf24_flush_rx();
    nrf24_flush_tx();

    NRF24_HAL_CE_Enable();
    NRF24_HAL_Delay(5);
}


void NRF24_TxMode(uint8_t *Address, uint8_t channel) {
    NRF24_HAL_CE_Disable();
    NRF24_HAL_Delay(5);

    nrf24_WriteReg(RF_CH, channel);
    nrf24_WriteRegMulti(TX_ADDR, Address, 5);
    nrf24_WriteRegMulti(RX_ADDR_P0, Address, 5);

    uint8_t config = nrf24_ReadReg(CONFIG);
    config &= ~(1 << 0);
    config |= (1 << 1);
    config |= (1 << 3);
    config |= (1 << 2);
    nrf24_WriteReg(CONFIG, config);

    NRF24_HAL_Delay(2);

    NRF24_HAL_CE_Enable();
    NRF24_HAL_Delay(1);
}

uint8_t NRF24_Transmit(uint8_t *data, uint8_t size) {
    uint8_t payload_to_send[32];
    uint8_t actual_size = size;

    if (actual_size == 0) return 0;
    if (actual_size > 32) actual_size = 32;


    memset(payload_to_send, 0, 32);
    memcpy(payload_to_send, data, actual_size);


    uint8_t cmd = W_TX_PAYLOAD;
    NRF24_HAL_CS_Select();
    NRF24_HAL_SPI_Transmit(&cmd, 1, 100);
    NRF24_HAL_SPI_Transmit(payload_to_send, 32, 1000);
    NRF24_HAL_CS_UnSelect();


    uint32_t start_tick = NRF24_HAL_GetTick();
    uint8_t status_reg;
    while (1) {
        status_reg = nrf24_ReadReg(STATUS);

        if (status_reg & ((1 << TX_DS_BIT) | (1 << MAX_RT_BIT))) {
            break;
        }
        if (NRF24_HAL_GetTick() - start_tick > 200) {
            nrf24_flush_tx();
            return 0;
        }
    }



    nrf24_WriteReg(STATUS, status_reg | (1 << TX_DS_BIT) | (1 << MAX_RT_BIT));

    if (status_reg & (1 << TX_DS_BIT)) {
        return 1;

    } else if (status_reg & (1 << MAX_RT_BIT)) {
        nrf24_flush_tx();
        return 0;
    }
    return 0;
}


void NRF24_RxMode(uint8_t *AddressPipe1, uint8_t AddressPipe2LSB, uint8_t channel) {
    NRF24_HAL_CE_Disable();
    NRF24_HAL_Delay(5);

    nrf24_clear_interrupts();
    nrf24_flush_rx();
    nrf24_flush_tx();

    nrf24_WriteReg(RF_CH, channel);
    nrf24_WriteReg(EN_RXADDR, (1 << 0) | (1 << 1) | (1 << 2));
    nrf24_WriteRegMulti(RX_ADDR_P1, AddressPipe1, 5);
    nrf24_WriteReg(RX_ADDR_P2, AddressPipe2LSB);


    uint8_t config = nrf24_ReadReg(CONFIG);
    config |= (1 << 0);
    config |= (1 << 1);
    config |= (1 << 3);
    config |= (1 << 2);
    nrf24_WriteReg(CONFIG, config);

    NRF24_HAL_Delay(2);

    NRF24_HAL_CE_Enable();
    NRF24_HAL_Delay(1);
}

uint8_t isDataAvailable(uint8_t pipenum) {
    uint8_t status = nrf24_ReadReg(STATUS);
    if ((status & (1 << RX_DR_BIT)) && (((status & RX_P_NO_MASK) >> RX_P_NO_POS) == pipenum)) {
        return 1;
    }
    return 0;
}

void NRF24_Receive(uint8_t *data) {
    uint8_t cmdtosend;

    NRF24_HAL_CS_Select();
    cmdtosend = R_RX_PAYLOAD;
    NRF24_HAL_SPI_Transmit(&cmdtosend, 1, 100);
    NRF24_HAL_SPI_Receive(data, 32, 1000);
    NRF24_HAL_CS_UnSelect();

    uint8_t status_reg = nrf24_ReadReg(STATUS);
    nrf24_WriteReg(STATUS, status_reg | (1 << RX_DR_BIT));

}


void NRF24_ReadAll(uint8_t *data) {
    uint8_t j = 0;

    for (int i = 0; i <= 0x09; i++) {
        *(data + j++) = nrf24_ReadReg(i);
    }

    nrf24_ReadReg_Multi(RX_ADDR_P0, (data + j), 5);
    j += 5;

    nrf24_ReadReg_Multi(RX_ADDR_P1, (data + j), 5);
    j += 5;

    *(data + j++) = nrf24_ReadReg(RX_ADDR_P2);
    *(data + j++) = nrf24_ReadReg(RX_ADDR_P3);
    *(data + j++) = nrf24_ReadReg(RX_ADDR_P4);
    *(data + j++) = nrf24_ReadReg(RX_ADDR_P5);

    nrf24_ReadReg_Multi(TX_ADDR, (data + j), 5);
    j += 5;

    for (int i = 0x11; i <= 0x16; i++) {
        *(data + j++) = nrf24_ReadReg(i);
    }

    *(data + j++) = nrf24_ReadReg(FIFO_STATUS);
    *(data + j++) = nrf24_ReadReg(DYNPD);
    *(data + j++) = nrf24_ReadReg(FEATURE);

}
