// See LICENSE for license details.

#ifndef _TOUDAI_SERIALPUSHER_H
#define _TOUDAI_SERIALPUSHER_H

/* Register offsets */

#define SERIALPUSHER_REG_SCKDIV          0x00
#define SERIALPUSHER_REG_SCKMODE         0x04
#define SERIALPUSHER_REG_CSID            0x10
#define SERIALPUSHER_REG_CSDEF           0x14
#define SERIALPUSHER_REG_CSMODE          0x18

#define SERIALPUSHER_REG_DCSSCK          0x28
#define SERIALPUSHER_REG_DSCKCS          0x2a
#define SERIALPUSHER_REG_DINTERCS        0x2c
#define SERIALPUSHER_REG_DINTERXFR       0x2e

#define SERIALPUSHER_REG_FMT             0x40
#define SERIALPUSHER_REG_TXCTRL          0x50
#define SERIALPUSHER_REG_RXCTRL          0x54

#define SERIALPUSHER_REG_IE              0x70
#define SERIALPUSHER_REG_IP              0x74

#define SERIALPUSHER_REG_RXSTAT          0x80
#define SERIALPUSHER_REG_TXSTAT          0x84
#define SERIALPUSHER_REG_RXFIFO          0x100
#define SERIALPUSHER_REG_TXFIFO          0x200

/* Fields */

#define SERIALPUSHER_SCK_POL             0x1
#define SERIALPUSHER_SCK_PHA             0x2

#define SERIALPUSHER_FMT_PROTO(x)        ((x) & 0x3)
#define SERIALPUSHER_FMT_ENDIAN(x)       (((x) & 0x1) << 2)
#define SERIALPUSHER_FMT_DIR(x)          (((x) & 0x1) << 3)
#define SERIALPUSHER_FMT_LEN(x)          (((x) & 0xf) << 16)

/* TXCTRL register */
#define SERIALPUSHER_TXWM(x)             ((x) & 0xffff)
/* RXCTRL register */
#define SERIALPUSHER_RXWM(x)             ((x) & 0xffff)

#define SERIALPUSHER_IP_TXWM             0x1
#define SERIALPUSHER_IP_RXWM             0x2

#define SERIALPUSHER_FCTRL_EN            0x1

#define SERIALPUSHER_INSN_CMD_EN         0x1
#define SERIALPUSHER_INSN_ADDR_LEN(x)    (((x) & 0x7) << 1)
#define SERIALPUSHER_INSN_PAD_CNT(x)     (((x) & 0xf) << 4)
#define SERIALPUSHER_INSN_CMD_PROTO(x)   (((x) & 0x3) << 8)
#define SERIALPUSHER_INSN_ADDR_PROTO(x)  (((x) & 0x3) << 10)
#define SERIALPUSHER_INSN_DATA_PROTO(x)  (((x) & 0x3) << 12)
#define SERIALPUSHER_INSN_CMD_CODE(x)    (((x) & 0xff) << 16)
#define SERIALPUSHER_INSN_PAD_CODE(x)    (((x) & 0xff) << 24)

#define SERIALPUSHER_TXFIFO_FULL  (1 << 31)
#define SERIALPUSHER_RXFIFO_EMPTY (1 << 31)

/* Values */

#define SERIALPUSHER_CSMODE_AUTO         0
#define SERIALPUSHER_CSMODE_HOLD         2
#define SERIALPUSHER_CSMODE_OFF          3

#define SERIALPUSHER_DIR_RX              0
#define SERIALPUSHER_DIR_TX              1

#define SERIALPUSHER_PROTO_S             0
#define SERIALPUSHER_PROTO_D             1
#define SERIALPUSHER_PROTO_Q             2

#define SERIALPUSHER_ENDIAN_MSB          0
#define SERIALPUSHER_ENDIAN_LSB          1

#endif /* _TOUDAI_SERIALPUSHER_H */
