#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <windows.h>
void fm_log(const char* format, ...)
{
	FILE *fp = NULL;
	fopen_s(&fp, "fm.log", "ab");
	if (NULL == fp)
		return;

	time_t tNow;
	time(&tNow);
	struct tm tt;
	localtime_s(&tt, &tNow);
	fprintf(fp, "[%4d-%02d-%02d %02d:%02d:%02d][pid:% 5d]",
		tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday,
		tt.tm_hour, tt.tm_min, tt.tm_sec, HandleToLong(GetCurrentThread()));

	va_list args;
	va_start(args, format);
	vfprintf(fp, format, args);
	va_end(args);
	fprintf(fp, "\t\n");
	fclose(fp);
}