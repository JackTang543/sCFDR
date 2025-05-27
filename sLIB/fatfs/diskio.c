/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include "stm32f4xx_hal.h"

extern SD_HandleTypeDef hsd;


#define DEV_SDCARD	0
// #define STORAGE_BLK_NBR                  (62333952) // 32GB
// #define STORAGE_BLK_SIZ                  (512)

static uint32_t blk_number = 62333952 - 1; //默认32GB
static uint32_t blk_size = 512; //默认512字节
#include "sightseerUtils/sUtils.h"


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{

	if(pdrv == DEV_SDCARD) {
		if(HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER){
			log_printfln("disk_status: SD卡已准备好,状态=%d",HAL_SD_GetCardState(&hsd));
			return 0;

		}else{
			log_error("disk_status: SD卡未准备好,状态=%d",HAL_SD_GetCardState(&hsd));
			return STA_NOINIT;
		}
	}else{
		log_error("disk_status: 无效的物理驱动号 %d", pdrv);
		return STA_NOINIT;
	}
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if(pdrv == DEV_SDCARD) {
		//读取logic block number和logic block size,这样就可以自动适应不同大小的SD卡
		blk_number = hsd.SdCard.LogBlockNbr - 1;
		blk_size = hsd.SdCard.LogBlockSize;
		log_printfln("SD卡初始化成功,逻辑块数量=%u,逻辑块大小=%u",blk_number,blk_size);

		return 0;
	}else{
		log_error("disk_initialize: 无效的物理驱动号 %d", pdrv);
		return STA_NOINIT;
	}
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if(pdrv != DEV_SDCARD){
		log_error("disk_read: 无效的物理驱动号 %d", pdrv);
		return RES_PARERR;
	}

	if(HAL_SD_ReadBlocks(&hsd, (uint8_t*)buff, sector, count, 2000) != HAL_OK){
		log_error("读取SD卡失败,sector=%u,count=%u",sector,count);
		return RES_ERROR;
	}else{
		log_printfln("读取SD卡成功,sector=%u,count=%u",sector,count);
		return RES_OK;
	}

}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	if(pdrv != DEV_SDCARD){
		log_error("disk_write: 无效的物理驱动号 %d", pdrv);
		return RES_PARERR;
	}


	{
		HAL_SD_CardStateTypeDef status = HAL_SD_GetCardState(&hsd);
		while(HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER){
			log_printfln("等待SD卡写入完成...");
		}

	}
	//先擦除
	{
		HAL_StatusTypeDef status = HAL_SD_Erase(&hsd, sector, sector + count);
		if(status != HAL_OK){
			log_error("擦除SD卡失败,sector=%u,count=%u,sta=%u",sector,count,status);
			return RES_ERROR;
		}
	}
	while(HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER){
		log_printfln("等待SD卡擦除完成...");
	}

	HAL_StatusTypeDef status = HAL_SD_GetCardState(&hsd);
	if(status != HAL_SD_CARD_TRANSFER){
		log_error("SD卡擦除失败,状态=%d",status);
		return RES_ERROR;
	}

	status = HAL_SD_WriteBlocks(&hsd, (uint8_t*)buff, sector, count, 2000);
	if(status != HAL_OK){
		log_error("写入SD卡失败,sector=%u,count=%u,sta=%u",sector,count,status);
		return RES_ERROR;
	}
	log_printfln("写入SD卡成功,sector=%u,count=%u",sector,count);

	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;


	if(pdrv != DEV_SDCARD){
		return RES_PARERR;
	}

	switch(cmd) {
		case CTRL_SYNC :	/* Make sure that no pending write process */
			res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			*(DWORD*)buff = blk_number;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			*(DWORD*)buff = 1;
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE :	/* Get sector size (WORD) */
			*(WORD*)buff = blk_size;
			res = RES_OK;
			break;

		default:
			res = RES_PARERR;
			break;
	}

	return res;
}

