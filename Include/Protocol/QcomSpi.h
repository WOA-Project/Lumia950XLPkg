#ifndef __QCOM_SPI_H__
#define __QCOM_SPI_H__

#define QCOM_SPI_QUP_PROTOCOL_GUID                                             \
  {                                                                            \
    0x5fd394e8, 0x3f10, 0x4be5,                                                \
    {                                                                          \
      0xba, 0x3e, 0x12, 0x78, 0xab, 0x5e, 0x01, 0x9a                           \
    }                                                                          \
  }

// Slightly modified because we do not support BAM DMA
struct spi_transfer {
  /* it's ok if tx_buf == rx_buf (right?)
   * for MicroWire, one buffer must be null
   * buffers must work with dma_*map_single() calls, unless
   *   spi_message.is_dma_mapped reports a pre-existing mapping
   */
  const void *tx_buf;
  void *      rx_buf;
  unsigned    len;

  unsigned cs_change : 1;
  u8       bits_per_word;
  u16      delay_usecs;
  u32      speed_hz;
};

typedef struct _QCOM_SPI_QUP_PROTOCOL QCOM_SPI_QUP_PROTOCOL;
typedef EFI_STATUS (EFIAPI *spi_qup_xfer_t)(struct spi_transfer *xfer);

struct _QCOM_SPI_QUP_PROTOCOL {
  spi_qup_xfer_t Transfer;
};

extern EFI_GUID gQcomSpiQupProtocolGuid;

#endif
