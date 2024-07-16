/**
 * @file lora_driver.h
 * @brief LoRa driver interface for ESP32
 *
 * Provides an interface to control a LoRa device using ESP32. Includes initialization,
 * configuration, and data transmission/reception functionalities.
 */

#ifndef LORA_DRIVER_H
#define LORA_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "lora_driver_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** @brief Implicit header mode indicator. */
    extern uint8_t __implicit;
    /** @brief Frequency of the LoRa module. */
    extern long __frequency;
    /** @brief Counter for lost packets during send operation. */
    extern uint8_t __send_packet_lost;

    /**
     * @brief Write a value to a register.
     * @param reg Register index.
     * @param val Value to write.
     * @return lora_status_t Result of the SPI write operation.
     */
    lora_status_t lora_write_reg(uint8_t reg, uint8_t val);

    /**
     * @brief Write a buffer to a register.
     * @param reg Register index.
     * @param val Pointer to the buffer to write.
     * @param len Byte length to write.
     * @return lora_status_t Result of the SPI write operation.
     */
    lora_status_t lora_write_reg_buffer(uint8_t reg, uint8_t *val, uint8_t len);

    /**
     * @brief Read the current value of a register.
     * @param reg Register index.
     * @param val Pointer to store the read value.
     * @return lora_status_t Result of the SPI read operation.
     */
    lora_status_t lora_read_reg(uint8_t reg, uint8_t *val);

    /**
     * @brief Read the current value of a register into a buffer.
     * @param reg Register index.
     * @param val Buffer to store the read value(s).
     * @param len Byte length to read.
     * @return lora_status_t Result of the SPI read operation.
     */
    lora_status_t lora_read_reg_buffer(uint8_t reg, uint8_t *val, uint8_t len);

    /**
     * @brief Configure explicit header mode.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_explicit_header_mode(void);

    /**
     * @brief Configure implicit header mode.
     * @param size Size of the packets.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_implicit_header_mode(uint8_t size);

    /**
     * @brief Sets the radio transceiver in idle mode.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_idle_mode(void);

    /**
     * @brief Sets the radio transceiver in sleep mode.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_sleep_mode(void);

    /**
     * @brief Sets the radio transceiver in receive mode.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_receive_mode(void);

    /**
     * @brief Configure power level for transmission.
     * @param level Power level (2-17, from least to most power).
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_tx_power(uint8_t level);

    /**
     * @brief Set carrier frequency.
     * @param frequency Frequency in Hz.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_frequency(long frequency);

    /**
     * @brief Set spreading factor.
     * @param sf Spreading factor (6-12).
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_spreading_factor(uint8_t sf);

    /**
     * @brief Get spreading factor.
     * @param sf Pointer to store the spreading factor.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_get_spreading_factor(uint8_t *sf);

    /**
     * @brief Set mapping of pins DIO0 to DIO5.
     * @param dio Number of DIO (0 to 5).
     * @param mode Mode of DIO (0 to 3).
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_dio_mapping(uint8_t dio, uint8_t mode);

    /**
     * @brief Get mapping of pins DIO0 to DIO5.
     * @param dio Number of DIO (0 to 5).
     * @param mapping Pointer to store mapping mode.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_get_dio_mapping(uint8_t dio, uint8_t *mapping);

    /**
     * @brief Set bandwidth (bit rate).
     * @param sbw Signal bandwidth (0 to 9).
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_bandwidth(uint8_t sbw);

    /**
     * @brief Get bandwidth (bit rate).
     * @param sbw Pointer to store the signal bandwidth.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_get_bandwidth(uint8_t *sbw);

    /**
     * @brief Set coding rate.
     * @param denominator Denominator for the coding rate 4/x.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_coding_rate(uint8_t denominator);

    /**
     * @brief Get coding rate.
     * @param cr Pointer to store coding rate.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_get_coding_rate(uint8_t *cr);

    /**
     * @brief Set the size of preamble.
     * @param length Preamble length in symbols.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_preamble_length(long length);

    /**
     * @brief Get the size of preamble.
     * @param preamble Pointer to store preamble length.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_get_preamble_length(long *preamble);

    /**
     * @brief Change radio sync word.
     * @param sw New sync word to use.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_set_sync_word(uint8_t sw);

    /**
     * @brief Enable appending/verifying packet CRC.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_enable_crc(void);

    /**
     * @brief Disable appending/verifying packet CRC.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_disable_crc(void);

    /**
     * @brief Perform hardware initialization.
     * @return lora_status_t Result of initialization.
     */
    lora_status_t lora_driver_init(void);

    /**
     * @brief Send a packet.
     * @param buf Data to be sent.
     * @param size Size of data.
     * @return lora_status_t Result of send operation.
     */
    lora_status_t lora_send_packet(uint8_t *buf, uint8_t size);

    /**
     * @brief Read a received packet.
     * @param buf Buffer for the data.
     * @param return_len Pointer to store the number of bytes received.
     * @param size Available size in buffer (bytes).
     * @return lora_status_t Result of receive operation.
     */
    lora_status_t lora_receive_packet(uint8_t *buf, uint8_t *return_len, uint8_t size);

    /**
     * @brief Check if there is data to read (packet received).
     * @param received Pointer to store the received status.
     * @param crc_error Pointer to store the CRC error status.
     * @return lora_status_t Result of check.
     */
    lora_status_t lora_received(bool *received, bool *crc_error);

    /**
     * @brief Returns RegIrqFlags.
     * @param irq_flags Pointer to store IRQ flags.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_get_irq(uint8_t *irq_flags);

    /**
     * @brief Return lost send packet count.
     * @return Number of lost packets.
     */
    uint8_t lora_packet_lost(void);

    /**
     * @brief Return last packet's RSSI.
     * @param rssi Pointer to store RSSI value.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_packet_rssi(uint8_t *rssi);

    /**
     * @brief Return last packet's SNR (signal to noise ratio).
     * @param snr Pointer to store SNR value.
     * @return lora_status_t Result of operation.
     */
    lora_status_t lora_packet_snr(uint8_t *snr);

    /**
     * @brief Shutdown hardware.
     */
    void lora_close(void);

    /**
     * @brief Dump LoRa registers for debugging.
     * @return lora_status_t Result of dump operation.
     */
    lora_status_t lora_dump_registers(void);

#ifdef __cplusplus
}
#endif

#endif // LORA_DRIVER_H