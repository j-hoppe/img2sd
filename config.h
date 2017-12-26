/* config.h: Parses the SCSI2SD config file XML with libxml2

 Copyright (c) 2017, Joerg Hoppe
 j_hoppe@t-online.de, www.retrocmp.com

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 JOERG HOPPE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 26-Dec-2017	JH Published
 15-May-2017	JH Created
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
