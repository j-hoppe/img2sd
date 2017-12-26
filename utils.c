/*
 * utils.c
 *
 *  Created on: 15.05.2017
 *      Author: root
 */
#include <time.h>

char *cur_time_text() {
	static char result[40];
	time_t timer;
	struct tm* tm_info;
	time(&timer);
	tm_info = localtime(&timer);
	strftime(result, 26, "%H:%M:%S", tm_info);
	return result;
}
