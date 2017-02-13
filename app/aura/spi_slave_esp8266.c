#include <stdio.h>

#include "spi_slave.h"
#include "app_error.h"
#include "boards.h"
#include <common.h>

#define TX_BUF_SIZE   5u               /**< SPI TX buffer size. */
#define RX_BUF_SIZE   TX_BUF_SIZE       /**< SPI RX buffer size. */
#define DEF_CHARACTER 0xAAu             /**< SPI default character. Character clocked out in case of an ignored transaction. */
#define ORC_CHARACTER 0x55u             /**< SPI over-read character. Character clocked out after an over-read of the transmit buffer. */

static uint8_t m_tx_buf[TX_BUF_SIZE];   /**< SPI TX buffer. */
static uint8_t m_rx_buf[RX_BUF_SIZE];   /**< SPI RX buffer. */

/**@brief Function for initializing buffers.
 *
 * @param[in] p_tx_buf  Pointer to a transmit buffer.
 * @param[in] p_rx_buf  Pointer to a receive  buffer.
 * @param[in] len       Buffers length.
 */
static inline void spi_slave_buffers_init(uint8_t * const p_tx_buf, uint8_t * const p_rx_buf, const uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        p_tx_buf[i] = (uint8_t)('a' + i);
        p_rx_buf[i] = 0;
    }
}

/**@brief Function for SPI slave event callback.
 *
 * Upon receiving an SPI transaction complete event, LED1 will blink and the buffers will be set.
 *
 * @param[in] event SPI slave driver event.
 */
static void spi_slave_event_handle(spi_slave_evt_t event)
{
    uint32_t err_code;

    if (event.evt_type == SPI_SLAVE_XFER_DONE)
    {
        //Check if received data is valid.
        printf("SPI data: ");
        for (int i = 0; i < RX_BUF_SIZE; i++) {
            printf("%c (%d) ", m_rx_buf[i], m_rx_buf[i]);
            m_rx_buf[i] = 0;
            m_tx_buf[i] = 'A' + i;
        }
        printf("\n");

        //Set buffers.
        err_code = spi_slave_buffers_set(m_tx_buf, m_rx_buf, sizeof(m_tx_buf), sizeof(m_rx_buf));
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for initializing SPI slave.
 *
 *  Function configures a SPI slave and sets buffers.
 *
 * @retval NRF_SUCCESS  Initialization successful.
 */
uint32_t spi_slave_esp8266_init(void)
{
    uint32_t           err_code;
    spi_slave_config_t spi_slave_config;

    err_code = spi_slave_evt_handler_register(spi_slave_event_handle);
    APP_ERROR_CHECK(err_code);

    spi_slave_config.pin_miso         = SPIS_MISO_PIN;
    spi_slave_config.pin_mosi         = SPIS_MOSI_PIN;
    spi_slave_config.pin_sck          = SPIS_SCK_PIN;
    spi_slave_config.pin_csn          = SPIS_CSN_PIN;
    spi_slave_config.mode             = SPI_MODE_0;
    spi_slave_config.bit_order        = SPIM_LSB_FIRST;
    spi_slave_config.def_tx_character = DEF_CHARACTER;
    spi_slave_config.orc_tx_character = ORC_CHARACTER;

    err_code = spi_slave_init(&spi_slave_config);
    APP_ERROR_CHECK(err_code);

    //Initialize buffers.
    spi_slave_buffers_init(m_tx_buf, m_rx_buf, (uint16_t)TX_BUF_SIZE);

    //Set buffers.
    err_code = spi_slave_buffers_set(m_tx_buf, m_rx_buf, sizeof(m_tx_buf), sizeof(m_rx_buf));
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}
