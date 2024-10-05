/*
 * sd.c
 *
 *  Created on: Mar 21, 2024
 *      Author: cepa
 */

#include <stdio.h>
#include "cmsis_os.h"
#include "debug.h"
#include "sd.h"
#include "report.h"
#include "printf.h"

QueueHandle_t sdQueue;

void SD_TaskHandler(void const *argument)
{
	FATFS FatFs;   // FATFS handle
	FRESULT fres;  // Common result code
	FILINFO fno;	  // Structure holds information
	FATFS *getFreeFs; 	  // Read information
	FIL fil;
	DIR dir;			  // Directory object structure
	DWORD free_clusters;  // Free Clusters
	DWORD free_sectors;	  // Free Sectors
	DWORD total_sectors;  // Total Sectors

	Report_t report;
	char filename[16];
	int i = 0, n = 0;

	fres = f_mount(&FatFs, "", 1); // 1 -> Mount now
	if (fres == FR_OK) {
		// Get some statistics from the SD card
		fres = f_getfree("", &free_clusters, &getFreeFs);

		// Formula comes from ChaN's documentation
		total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
		free_sectors = free_clusters * getFreeFs->csize;

	    _DEBUG("SD card mounted: total=%luMiB free=%luMiB", total_sectors / 2 / 1024, free_sectors / 2 / 1024);

	} else {
		_DEBUG("SD card fail");
		return;
	}

	for (;;) {
		while (uxQueueMessagesWaiting(sdQueue) > 0) {
			if (xQueueReceive(sdQueue, &report, 100) == pdTRUE) {
				//DebugReport(&report);

				// Daily filename
				PrintfLock();
				snprintf(filename, 16, "%02d%02d%02d.log",
						report.date.year, report.date.month, report.date.day);
				PrintfUnlock();

				// Write to daily file
				f_open(&fil, filename, FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE | FA_READ);
				f_puts(ReportToIridiumPayload(&report), &fil);
				f_close(&fil);

				_DEBUG("Report written to file: %s", filename);
			}
		}

		osDelay(10000);
	}
}
