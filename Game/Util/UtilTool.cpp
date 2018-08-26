#include "../Util/UtilTool.h"
#include "../Tool/UtilString.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <windows.h>
void fm_log(const char* szFormat, ...)
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
	va_start(args, szFormat);
	vfprintf(fp, szFormat, args);
	va_end(args);
	fprintf(fp, "\t\n");
	fclose(fp);
}

void trace_log(const char* szFormat, ...)
{
#if _DEBUG
	FILE *fp = NULL;
	fopen_s(&fp, "trace.log", "ab");

	time_t tNow;
	time(&tNow);
	struct tm tt;
	localtime_s(&tt, &tNow);
	if (nullptr != fp)
	{
		fprintf(fp, "[%4d-%02d-%02d %02d:%02d:%02d][pid:% 5d]",
			tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday,
			tt.tm_hour, tt.tm_min, tt.tm_sec, HandleToLong(GetCurrentThread()));

		va_list args;
		va_start(args, szFormat);
		vfprintf(fp, szFormat, args);
		va_end(args);
		fprintf(fp, "\t\n");
		fclose(fp);
	}

	fprintf(stderr, "[%4d-%02d-%02d %02d:%02d:%02d][pid:% 5d]",
		tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday,
		tt.tm_hour, tt.tm_min, tt.tm_sec, HandleToLong(GetCurrentThread()));

	va_list args;
	va_start(args, szFormat);
	vfprintf(stderr, szFormat, args);
	va_end(args);
	fprintf(stderr, "\t\n");
#endif
}