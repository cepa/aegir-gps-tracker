/*
 * report.c
 *
 *  Created on: Apr 24, 2024
 *      Author: cepa
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "debug.h"
#include "report.h"
#include "gps.h"
#include "printf.h"

uint32_t lastReportSendTime = 0;

float _coordToNMEA(float coord)
{
	int sig = (int) coord;
	float dec = coord - (float) sig;
	return sig * 100.0 + dec * 60.0;
}

void DebugReport(Report_t *report)
{
	assert(report != NULL);

	_DEBUG("Report: %02d%02d%02d%02d%02d%02d,%09.4f,%c,%010.4f,%c,%0.2f,%0.2f",
			report->date.year, report->date.month, report->date.day,
			report->time.hours, report->time.minutes, report->time.seconds,
			_coordToNMEA(report->latitude), report->latitude > 0.0 ? 'N' : 'S',
			_coordToNMEA(report->longitude), report->longitude > 0.0 ? 'E' : 'W',
			report->speed > 0.0 ? report->speed : 0.0,
			report->course > 0.0 ? report->course : 0.0);

}

int PrepareReport(Report_t *report)
{
	assert(report != NULL);
	memset(report, 0, sizeof (Report_t));

	report->date.year		= GPS->date.year;
	report->date.month		= GPS->date.month;
	report->date.day		= GPS->date.day;

	report->time.hours		= GPS->time.hours;
	report->time.minutes	= GPS->time.minutes;
	report->time.seconds	= GPS->time.seconds;

	report->latitude		= minmea_tocoord(&GPS->latitude);
	report->longitude		= minmea_tocoord(&GPS->longitude);
	report->speed			= minmea_tofloat(&GPS->speed);
	report->course			= minmea_tofloat(&GPS->course);
	report->altitude		= minmea_tofloat(&GPS->altitude);

	if (isnan(report->latitude) || isnan(report->longitude)) {
		return REPORT_ERROR;
	}

	return REPORT_OK;
}

const char *ReportToCobanPayload(Report_t *report)
{
	static char payload[128];
	assert(report != NULL);

	PrintfLock();
	snprintf(payload, 128, "imei:868822047673593,tracker,%02d%02d%02d%02d%02d%02d,,F,%010.3f,A,%09.4f,%c,%010.4f,%c,%0.2f,%0.2f;\n",
			report->date.year, report->date.month, report->date.day,
			report->time.hours, report->time.minutes, report->time.seconds,
			report->time.hours * 10000.0 + report->time.minutes * 100.0 + report->time.seconds,
			_coordToNMEA(report->latitude), report->latitude > 0.0 ? 'N' : 'S',
			_coordToNMEA(report->longitude), report->longitude > 0.0 ? 'E' : 'W',
			report->speed > 0.0 ? report->speed : 0.0,
			report->course > 0.0 ? report->course : 0.0);
	PrintfUnlock();

	return payload;
}

const char *ReportToIridiumPayload(Report_t *report)
{
	static char payload[50];
	assert(report != NULL);

	PrintfLock();
	snprintf(payload, 50, "%02d%02d%02d%02d%02d%02d,%0.4f,%0.4f,%0.2f,%d,%d",
			report->date.year, report->date.month, report->date.day,
			report->time.hours, report->time.minutes, report->time.seconds,
			report->latitude,
			report->longitude,
			report->speed > 0.0 ? report->speed : 0.0,
			report->course > 0.0 ? (int) report->course : 0,
			report->altitude > 0.0 ? (int) report->altitude : 0);
	PrintfUnlock();

	return payload;
}
