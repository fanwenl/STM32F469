/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
#include "bsp_sd.h"		/* Example: Header file of existing MMC/SDC contorl module */

/* Definitions of physical drive number for each drive */
#define ATA		1	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */

#define BLOCKSIZE 512

static volatile DSTATUS Stat = STA_NOINIT;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case ATA :
//		result = ATA_disk_status();

		// translate the reslut code here

		return stat;

	case MMC :
		stat = STA_NOINIT;
		if(BSP_SD_GetStatus() == SD_TRANSFER_OK)
		{
			stat &= ~STA_NOINIT;
		}

		// translate the reslut code here

		return stat;

	case USB :
//		result = USB_disk_status();

		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case ATA :
//		result = ATA_disk_initialize();

		// translate the reslut code here

		return stat;

	case MMC :
		stat = STA_NOINIT;
		if(BSP_SD_Init() == MSD_OK)
		{
			stat &= ~STA_NOINIT;
		}

		// translate the reslut code here

		return stat;

	case USB :
//		result = USB_disk_initialize();

		// translate the reslut code here

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

//		result = ATA_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here
		res = RES_OK; 
		if(BSP_SD_ReadBlocks_DMA((uint32_t *)buff, (uint64_t)(sector * BLOCKSIZE), BLOCKSIZE, count) != MSD_OK)
		{
			res = RES_ERROR;
		}

		// translate the reslut code here

		return res;

	case USB :
		// translate the arguments here

//		res = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

//		result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here
		res = RES_OK;
		if(BSP_SD_WriteBlocks((uint32_t *)buff, (uint64_t)(sector * BLOCKSIZE), BLOCKSIZE, count) !=MSD_OK)
		{
			res = RES_ERROR;
		}

		return res;

	case USB :
		// translate the arguments here

//		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :

		// Process of the command for the ATA drive

		return res;

	case MMC :
		// Process of the command for the MMC/SD card
		res = RES_ERROR;

		SD_CardInfo CardInfo;
  
//		if (Stat & STA_NOINIT) 

//			return RES_NOTRDY;

		switch (cmd)
		{
			/* Make sure that no pending write process */
			case CTRL_SYNC :
			res = RES_OK;
			break;

			/* Get number of sectors on the disk (DWORD) */
			case GET_SECTOR_COUNT :
			BSP_SD_GetCardInfo(&CardInfo);
			*(DWORD*)buff = CardInfo.CardCapacity / BLOCKSIZE;
			res = RES_OK;
			break;

			/* Get R/W sector size (WORD) */
			case GET_SECTOR_SIZE :
			*(WORD*)buff = BLOCKSIZE;
			res = RES_OK;
			break;

			/* Get erase block size in unit of sector (DWORD) */
			case GET_BLOCK_SIZE :
			*(DWORD*)buff = BLOCKSIZE;
			res = RES_OK;
			break;

			default:
			res = RES_PARERR;
		}
		return res;

	case USB :

		// Process of the command the USB drive

		return res;
	}

	return RES_PARERR;
}
#endif
  