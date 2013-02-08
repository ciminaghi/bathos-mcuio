#ifndef __LPC21_SPI_H__
#define __LPC21_SPI_H__

/* SPI (Serial Peripheral Interface) */
#define REG_SPCR	(0xE0020000 / 4)
#define REG_SPSR	(0xE0020004 / 4)
#define REG_SPDR	(0xE0020008 / 4)
#define REG_SPCCR	(0xE002000C / 4)
#define REG_SPTCR	(0xE0020010 / 4)
#define REG_SPTSR	(0xE0020014 / 4)
#define REG_SPTOR	(0xE0020018 / 4)
#define REG_SPINT	(0xE002001C / 4)

#define REG_S0SPCR	(0xE0020000 / 4)
#define REG_S0SPSR 	(0xE0020004 / 4)
#define REG_S0SPDR 	(0xE0020008 / 4)
#define REG_S0SPCCR	(0xE002000C / 4)
#define REG_S0SPTCR	(0xE0020010 / 4)
#define REG_S0SPTSR	(0xE0020014 / 4)
#define REG_S0SPTOR	(0xE0020018 / 4)
#define REG_S0SPINT	(0xE002001C / 4)

/* SPI1 (Serial Peripheral Interface 1) */
#define REG_S1SPCR 	(0xE0030000 / 4)
#define REG_S1SPSR 	(0xE0030004 / 4)
#define REG_S1SPDR 	(0xE0030008 / 4)
#define REG_S1SPCCR	(0xE003000C / 4)
#define REG_S1SPTCR	(0xE0030010 / 4)
#define REG_S1SPTSR	(0xE0030014 / 4)
#define REG_S1SPTOR	(0xE0030018 / 4)
#define REG_S1SPINT	(0xE003001C / 4)

/* SSP Controller */
#define REG_SSPCR0 	(0xE0068000 / 4)
#define REG_SSPCR1 	(0xE0068004 / 4)
#define REG_SSPDR  	(0xE0068008 / 4)
#define REG_SSPSR  	(0xE006800C / 4)
#define REG_SSPCPSR	(0xE0068010 / 4)
#define REG_SSPIMSC	(0xE0068014 / 4)
#define REG_SSPRIS	(0xE0068018 / 4)
#define REG_SSPMIS 	(0xE006801C / 4)
#define REG_SSPICR 	(0xE0068020 / 4)

#endif /* __LPC21_SPI_H__ */
