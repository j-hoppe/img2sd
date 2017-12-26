/*
 * config.h
 *
 *  Created on: 15.05.2017
 *      Author: root
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>

#define MAX_SCSITARGETS	8

// entry for one SCSI target id
// reduced data set
typedef struct {
	int targetId; // SCSI id
	int enabled;
	int deviceType; // 0 for disk

	int sectorStart; // start position for this drive on SDcard, in sectors.

	// drive geometry settings
	int sectors;

	int bytesPerSector;
	int sectorsPerTrack; // info
	int headsPerCylinder; // info
	char vendor[256]; // info
	char prodId[256] ; // info
	char revision[256];
	char serial[256] ; //

} config_scsitarget_t;

#ifndef CONFIG_C_
// index by SCSI id
extern config_scsitarget_t config_scsitargets[MAX_SCSITARGETS];
#endif

int config_load(char *docname);
void config_print_scsitarget(FILE *fout, config_scsitarget_t *st) ;

#endif /* CONFIG_H_ */
