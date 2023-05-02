#ifndef __QUP_SPI_H__
#define __QUP_SPI_H__

#define QUP_CONFIG 0x0000
#define QUP_STATE 0x0004
#define QUP_IO_M_MODES 0x0008
#define QUP_SW_RESET 0x000c
#define QUP_OPERATIONAL 0x0018
#define QUP_ERROR_FLAGS 0x001c
#define QUP_ERROR_FLAGS_EN 0x0020
#define QUP_OPERATIONAL_MASK 0x0028
#define QUP_HW_VERSION 0x0030
#define QUP_MX_OUTPUT_CNT 0x0100
#define QUP_OUTPUT_FIFO 0x0110
#define QUP_MX_WRITE_CNT 0x0150
#define QUP_MX_INPUT_CNT 0x0200
#define QUP_MX_READ_CNT 0x0208
#define QUP_INPUT_FIFO 0x0218
#define SPI_CONFIG 0x0300
#define SPI_IO_CONTROL 0x0304
#define SPI_ERROR_FLAGS 0x0308
#define SPI_ERROR_FLAGS_EN 0x030c
/* QUP_CONFIG fields */
#define QUP_CONFIG_SPI_MODE (1 << 8)
#define QUP_CONFIG_CLOCK_AUTO_GATE BIT(13)
#define QUP_CONFIG_NO_INPUT BIT(7)
#define QUP_CONFIG_NO_OUTPUT BIT(6)
#define QUP_CONFIG_N 0x001f
/* QUP_STATE fields */
#define QUP_STATE_VALID BIT(2)
#define QUP_STATE_RESET 0
#define QUP_STATE_RUN 1
#define QUP_STATE_PAUSE 3
#define QUP_STATE_MASK 3
#define QUP_STATE_CLEAR 2
#define QUP_HW_VERSION_2_1_1 0x20010001
/* QUP_IO_M_MODES fields */
#define QUP_IO_M_PACK_EN BIT(15)
#define QUP_IO_M_UNPACK_EN BIT(14)
#define QUP_IO_M_INPUT_MODE_MASK_SHIFT 12
#define QUP_IO_M_OUTPUT_MODE_MASK_SHIFT 10
#define QUP_IO_M_INPUT_MODE_MASK (3 << QUP_IO_M_INPUT_MODE_MASK_SHIFT)
#define QUP_IO_M_OUTPUT_MODE_MASK (3 << QUP_IO_M_OUTPUT_MODE_MASK_SHIFT)
#define QUP_IO_M_OUTPUT_BLOCK_SIZE(x) (((x) & (0x03 << 0)) >> 0)
#define QUP_IO_M_OUTPUT_FIFO_SIZE(x) (((x) & (0x07 << 2)) >> 2)
#define QUP_IO_M_INPUT_BLOCK_SIZE(x) (((x) & (0x03 << 5)) >> 5)
#define QUP_IO_M_INPUT_FIFO_SIZE(x) (((x) & (0x07 << 7)) >> 7)
#define QUP_IO_M_MODE_FIFO 0
#define QUP_IO_M_MODE_BLOCK 1
#define QUP_IO_M_MODE_DMOV 2
#define QUP_IO_M_MODE_BAM 3
/* QUP_OPERATIONAL fields */
#define QUP_OP_MAX_INPUT_DONE_FLAG BIT(11)
#define QUP_OP_MAX_OUTPUT_DONE_FLAG BIT(10)
#define QUP_OP_IN_SERVICE_FLAG BIT(9)
#define QUP_OP_OUT_SERVICE_FLAG BIT(8)
#define QUP_OP_IN_FIFO_FULL BIT(7)
#define QUP_OP_OUT_FIFO_FULL BIT(6)
#define QUP_OP_IN_FIFO_NOT_EMPTY BIT(5)
#define QUP_OP_OUT_FIFO_NOT_EMPTY BIT(4)
/* QUP_ERROR_FLAGS and QUP_ERROR_FLAGS_EN fields */
#define QUP_ERROR_OUTPUT_OVER_RUN BIT(5)
#define QUP_ERROR_INPUT_UNDER_RUN BIT(4)
#define QUP_ERROR_OUTPUT_UNDER_RUN BIT(3)
#define QUP_ERROR_INPUT_OVER_RUN BIT(2)
/* SPI_CONFIG fields */
#define SPI_CONFIG_HS_MODE BIT(10)
#define SPI_CONFIG_INPUT_FIRST BIT(9)
#define SPI_CONFIG_LOOPBACK BIT(8)
/* SPI_IO_CONTROL fields */
#define SPI_IO_C_FORCE_CS BIT(11)
#define SPI_IO_C_CLK_IDLE_HIGH BIT(10)
#define SPI_IO_C_MX_CS_MODE BIT(8)
#define SPI_IO_C_CS_N_POLARITY_0 BIT(4)
#define SPI_IO_C_CS_SELECT(x) (((x)&3) << 2)
#define SPI_IO_C_CS_SELECT_MASK 0x000c
#define SPI_IO_C_TRISTATE_CS BIT(1)
#define SPI_IO_C_NO_TRI_STATE BIT(0)
/* SPI_ERROR_FLAGS and SPI_ERROR_FLAGS_EN fields */
#define SPI_ERROR_CLK_OVER_RUN BIT(1)
#define SPI_ERROR_CLK_UNDER_RUN BIT(0)
#define SPI_NUM_CHIPSELECTS 4
/* high speed mode is when bus rate is greater then 26MHz */
#define SPI_HS_MIN_RATE 26000000
#define SPI_MAX_RATE 50000000
#define SPI_DELAY_THRESHOLD 1
#define SPI_DELAY_RETRY 10

// https://elixir.bootlin.com/linux/latest/source/include/linux/spi/spi.h#L148

#define SPI_TX_DUAL 0x100    /* transmit with 2 wires */
#define SPI_TX_QUAD 0x200    /* transmit with 4 wires */
#define SPI_RX_DUAL 0x400    /* receive with 2 wires */
#define SPI_RX_QUAD 0x800    /* receive with 4 wires */
#define SPI_CS_WORD 0x1000   /* toggle cs after each word */
#define SPI_TX_OCTAL 0x2000  /* transmit with 8 wires */
#define SPI_RX_OCTAL 0x4000  /* receive with 8 wires */
#define SPI_3WIRE_HIZ 0x8000 /* high impedance turnaround */

typedef struct _spi_qup {
  UINTN      base;
  int        irq;
  int        in_fifo_sz;
  int        out_fifo_sz;
  int        in_blk_sz;
  int        out_blk_sz;
  EFI_STATUS error;
  int        w_size; /* bytes per SPI word */
  int        tx_bytes;
  int        rx_bytes;
  int        qup_v1;

  struct spi_transfer *xfer;
  volatile INTN        done;

  /* Device clocks */
  struct clk *iface_clk;
  struct clk *cclk;

  /* This should belongs to SPI master, but merged anyway. */
  u8 mode;
#define SPI_CPHA 0x01      /* clock phase */
#define SPI_CPOL 0x02      /* clock polarity */
#define SPI_MODE_0 (0 | 0) /* (original MicroWire) */
#define SPI_MODE_1 (0 | SPI_CPHA)
#define SPI_MODE_2 (SPI_CPOL | 0)
#define SPI_MODE_3 (SPI_CPOL | SPI_CPHA)
#define SPI_CS_HIGH 0x04   /* chipselect active high? */
#define SPI_LSB_FIRST 0x08 /* per-word bits-on-wire */
#define SPI_3WIRE 0x10     /* SI/SO signals shared */
#define SPI_LOOP 0x20      /* loopback mode */
#define SPI_NO_CS 0x40     /* 1 dev/bus, no chipselect */
#define SPI_READY 0x80     /* slave pulls low to pause */
} spi_qup, *pspi_qup;

// Stupid
#define BITS_PER_BYTE 8

/**
 * enum irqreturn
 * @IRQ_NONE        interrupt was not from this device
 * @IRQ_HANDLED     interrupt was handled by this device
 * @IRQ_WAKE_THREAD handler requests to wake the handler thread
 */
enum irqreturn {
  IRQ_NONE,
  IRQ_HANDLED,
  IRQ_WAKE_THREAD,
  IRQ_FAIL,
};

typedef enum irqreturn irqreturn_t;

EFI_STATUS spi_qup_set_state(pspi_qup controller, u32 state);
void spi_qup_fifo_read(pspi_qup controller, struct spi_transfer *xfer);
void spi_qup_fifo_write(pspi_qup controller, struct spi_transfer *xfer);
irqreturn_t qup_spi_interrupt(void *pdata);
EFI_STATUS spi_qup_io_config(pspi_qup controller, struct spi_transfer *xfer);
EFI_STATUS spi_qup_transfer_one(pspi_qup controller, struct spi_transfer *xfer);

#endif
