/*
 * report.h
 *
 *  Created on: Apr 24, 2024
 *      Author: cepa
 */

#ifndef INC_REPORT_H_
#define INC_REPORT_H_

#include <stdint.h>
#include "cmsis_os.h"

#define REPORT_ERROR	0
#define REPORT_OK		1

typedef struct ReportDate {
	uint8_t year;
	uint8_t month;
	uint8_t day;
} ReportDate_t;

typedef struct ReportTime {
	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
} ReportTime_t;

typedef struct Report {
	ReportDate_t date;
	ReportTime_t time;
	float latitude;
	float longitude;
	float speed;
	float course;
	float altitude;
} Report_t;

extern uint32_t lastReportSendTime;

void DebugReport(Report_t *report);
int PrepareReport(Report_t *report);
const char *ReportToCobanPayload(Report_t *report);
const char *ReportToIridiumPayload(Report_t *report);

#endif /* INC_REPORT_H_ */
