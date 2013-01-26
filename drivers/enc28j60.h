/*
 * Register list for enc28j60, adapted from header by Pascal Stang, 2005
 * and further modified by Guido Socher. This revision by A Rubini. GNU GPL V2
 */
#ifndef __HW_ENC28J60_H__
#define __HW_ENC28J60_H__

/* SPI command codes */
#define ENC28_CMD_READ_CTRL_REG	0x00
#define ENC28_CMD_READ_BUF_MEM	0x3A
#define ENC28_CMD_WRITE_CTRL_REG	0x40
#define ENC28_CMD_WRITE_BUF_MEM	0x7A
#define ENC28_CMD_BIT_FIELD_SET	0x80
#define ENC28_CMD_BIT_FIELD_CLR	0xA0
#define ENC28_CMD_SOFT_RESET	0xFF


/* ENC28J60 Control Registers
 * Control register definitions are a combination of address,
 * bank number, and Ethernet/MAC/PHY indicator bits.
 * - Register address        (bits 0-4)
 * - Bank number             (bits 5-6)
 * - MAC/PHY indicator       (bit 7)
 */
#define ENC28_MASK_ADDR		0x1F
#define ENC28_MASK_BANK		0x60
#define ENC28_MASK_SPRD		0x80

/* All-bank registers */
#define ENC28_IE		0x1B
#define ENC28_IE_INTIE			0x80
#define ENC28_IE_PKTIE			0x40
#define ENC28_IE_DMAIE			0x20
#define ENC28_IE_LINKIE			0x10
#define ENC28_IE_TXIE			0x08
#define ENC28_IE_WOLIE			0x04
#define ENC28_IE_TXERIE			0x02
#define ENC28_IE_RXERIE			0x01

#define ENC28_IR		0x1C
#define ENC28_IR_PKTIF			0x40
#define ENC28_IR_DMAIF			0x20
#define ENC28_IR_LINKIF			0x10
#define ENC28_IR_TXIF			0x08
#define ENC28_IR_WOLIF			0x04
#define ENC28_IR_TXERIF			0x02
#define ENC28_IR_RXERIF			0x01

#define ENC28_STAT		0x1D
#define ENC28_STAT_INT			0x80
#define ENC28_STAT_LATECOL		0x10
#define ENC28_STAT_RXBUSY		0x04
#define ENC28_STAT_TXABRT		0x02
#define ENC28_STAT_CLKRDY		0x01

#define ENC28_CON2		0x1E
#define ENC28_CON2_AUTOINC		0x80
#define ENC28_CON2_PKTDEC		0x40
#define ENC28_CON2_PWRSV		0x20
#define ENC28_CON2_VRPS			0x08

#define ENC28_CON1		0x1F
#define ENC28_CON1_TXRST		0x80
#define ENC28_CON1_RXRST		0x40
#define ENC28_CON1_DMAST		0x20
#define ENC28_CON1_CSUMEN		0x10
#define ENC28_CON1_TXRTS		0x08
#define ENC28_CON1_RXEN			0x04
#define ENC28_CON1_BSEL1		0x02
#define ENC28_CON1_BSEL0		0x01

/* Bank 0 registers */
#define ENC28_RDPTL		(0x00|0x00)
#define ENC28_RDPTH		(0x01|0x00)
#define ENC28_WRPTL		(0x02|0x00)
#define ENC28_WRPTH		(0x03|0x00)
#define ENC28_TXSTL		(0x04|0x00)
#define ENC28_TXSTH		(0x05|0x00)
#define ENC28_TXNDL		(0x06|0x00)
#define ENC28_TXNDH		(0x07|0x00)
#define ENC28_RXSTL		(0x08|0x00)
#define ENC28_RXSTH		(0x09|0x00)
#define ENC28_RXNDL		(0x0A|0x00)
#define ENC28_RXNDH		(0x0B|0x00)
#define ENC28_RXRDPTL		(0x0C|0x00)
#define ENC28_RXRDPTH		(0x0D|0x00)
#define ENC28_RXWRPTL		(0x0E|0x00)
#define ENC28_RXWRPTH		(0x0F|0x00)
#define ENC28_DMASTL		(0x10|0x00)
#define ENC28_DMASTH		(0x11|0x00)
#define ENC28_DMANDL		(0x12|0x00)
#define ENC28_DMANDH		(0x13|0x00)
#define ENC28_DMADSTL		(0x14|0x00)
#define ENC28_DMADSTH		(0x15|0x00)
#define ENC28_DMACSL		(0x16|0x00)
#define ENC28_DMACSH		(0x17|0x00)

/* Bank 1 registers */
#define ENC28_HT0		(0x00|0x20)
#define ENC28_HT1		(0x01|0x20)
#define ENC28_HT2		(0x02|0x20)
#define ENC28_HT3		(0x03|0x20)
#define ENC28_HT4		(0x04|0x20)
#define ENC28_HT5		(0x05|0x20)
#define ENC28_HT6		(0x06|0x20)
#define ENC28_HT7		(0x07|0x20)
#define ENC28_PMM0		(0x08|0x20)
#define ENC28_PMM1		(0x09|0x20)
#define ENC28_PMM2		(0x0A|0x20)
#define ENC28_PMM3		(0x0B|0x20)
#define ENC28_PMM4		(0x0C|0x20)
#define ENC28_PMM5		(0x0D|0x20)
#define ENC28_PMM6		(0x0E|0x20)
#define ENC28_PMM7		(0x0F|0x20)
#define ENC28_PMCSL		(0x10|0x20)
#define ENC28_PMCSH		(0x11|0x20)
#define ENC28_PMOL		(0x14|0x20)
#define ENC28_PMOH		(0x15|0x20)
#define ENC28_WOLIE		(0x16|0x20)
#define ENC28_WOLIR		(0x17|0x20)
#define ENC28_RXFCON		(0x18|0x20)
#define ENC28_RXFCON_UCEN		0x80
#define ENC28_RXFCON_ANDOR		0x40
#define ENC28_RXFCON_CRCEN		0x20
#define ENC28_RXFCON_PMEN		0x10
#define ENC28_RXFCON_MPEN		0x08
#define ENC28_RXFCON_HTEN		0x04
#define ENC28_RXFCON_MCEN		0x02
#define ENC28_RXFCON_BCEN		0x01
#define ENC28_PKTCNT		(0x19|0x20)

/* Bank 2 registers */
#define ENC28_MACON1		(0x00|0x40|0x80)
#define ENC28_MACON1_LOOPBK		0x10
#define ENC28_MACON1_TXPAUS		0x08
#define ENC28_MACON1_RXPAUS		0x04
#define ENC28_MACON1_PASSALL		0x02
#define ENC28_MACON1_MARXEN		0x01
#define ENC28_MACON2		(0x01|0x40|0x80)
#define ENC28_MACON2_MARST		0x80
#define ENC28_MACON2_RNDRST		0x40
#define ENC28_MACON2_MARXRST		0x08
#define ENC28_MACON2_RFUNRST		0x04
#define ENC28_MACON2_MATXRST		0x02
#define ENC28_MACON2_TFUNRST		0x01
#define ENC28_MACON3		(0x02|0x40|0x80)
#define ENC28_MACON3_PADCFG2		0x80
#define ENC28_MACON3_PADCFG1		0x40
#define ENC28_MACON3_PADCFG0		0x20
#define ENC28_MACON3_TXCRCEN		0x10
#define ENC28_MACON3_PHDRLEN		0x08
#define ENC28_MACON3_HFRMLEN		0x04
#define ENC28_MACON3_FRMLNEN		0x02
#define ENC28_MACON3_FULDPX		0x01
#define ENC28_MACON4		(0x03|0x40|0x80)
#define ENC28_MABBIPG		(0x04|0x40|0x80)
#define ENC28_MAIPGL		(0x06|0x40|0x80)
#define ENC28_MAIPGH		(0x07|0x40|0x80)
#define ENC28_MACLCON1		(0x08|0x40|0x80)
#define ENC28_MACLCON2		(0x09|0x40|0x80)
#define ENC28_MAMXFLL		(0x0A|0x40|0x80)
#define ENC28_MAMXFLH		(0x0B|0x40|0x80)
#define ENC28_MAPHSUP		(0x0D|0x40|0x80)
#define ENC28_MICON		(0x11|0x40|0x80)
#define ENC28_MICMD		(0x12|0x40|0x80)
#define ENC28_MICMD_MIISCAN		0x02
#define ENC28_MICMD_MIIRD		0x01
#define ENC28_MIREGADR		(0x14|0x40|0x80)
#define ENC28_MIWRL		(0x16|0x40|0x80)
#define ENC28_MIWRH		(0x17|0x40|0x80)
#define ENC28_MIRDL		(0x18|0x40|0x80)
#define ENC28_MIRDH		(0x19|0x40|0x80)

/* Bank 3 registers */
#define ENC28_MAADR1		(0x00|0x60|0x80)
#define ENC28_MAADR0		(0x01|0x60|0x80)
#define ENC28_MAADR3		(0x02|0x60|0x80)
#define ENC28_MAADR2		(0x03|0x60|0x80)
#define ENC28_MAADR5		(0x04|0x60|0x80)
#define ENC28_MAADR4		(0x05|0x60|0x80)
#define ENC28_BSTSD		(0x06|0x60)
#define ENC28_BSTCON		(0x07|0x60)
#define ENC28_BSTCSL		(0x08|0x60)
#define ENC28_BSTCSH		(0x09|0x60)
#define ENC28_MISTAT		(0x0A|0x60|0x80)
#define ENC28_MISTAT_NVALID		0x04
#define ENC28_MISTAT_SCAN		0x02
#define ENC28_MISTAT_BUSY		0x01
#define ENC28_REVID		(0x12|0x60)
#define ENC28_COCON		(0x15|0x60)
#define ENC28_FLOCON		(0x17|0x60)
#define ENC28_PAUSL		(0x18|0x60)
#define ENC28_PAUSH		(0x19|0x60)

/* PHY registers */
#define ENC28_PHCON1		0x00
#define ENC28_PHCON1_PRST		0x8000
#define ENC28_PHCON1_PLOOPBK		0x4000
#define ENC28_PHCON1_PPWRSV		0x0800
#define ENC28_PHCON1_PDPXMD		0x0100
#define ENC28_PHSTAT1		0x01
#define ENC28_PHSTAT1_PFDPX		0x1000
#define ENC28_PHSTAT1_PHDPX		0x0800
#define ENC28_PHSTAT1_LLSTAT		0x0004
#define ENC28_PHSTAT1_JBSTAT		0x0002
#define ENC28_PHHID1		0x02
#define ENC28_PHHID2		0x03
#define ENC28_PHCON2		0x10
#define ENC28_PHCON2_FRCLINK		0x4000
#define ENC28_PHCON2_TXDIS		0x2000
#define ENC28_PHCON2_JABBER		0x0400
#define ENC28_PHCON2_HDLDIS		0x0100
#define ENC28_PHSTAT2		0x11
#define ENC28_PHIE		0x12
#define ENC28_PHIR		0x13
#define ENC28_PHLCON		0x14

/* ENC28J60 Packet Control Byte Bit Definitions -- FIXME */
#define ENC28_PKTCTRL_PHUGEEN		0x08
#define ENC28_PKTCTRL_PPADEN		0x04
#define ENC28_PKTCTRL_PCRCEN		0x02
#define ENC28_PKTCTRL_POVERRIDE		0x01


#endif /* __HW_ENC28J60_H__ */
