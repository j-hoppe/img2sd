/* config.c: Parses the SCSI2SD config file XML with libxml2

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

 Parses the SCSI2SD config file XML with libxml2
 See http://xmlsoft.org/example.html

 */
#define CONFIG_C_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "error.h"
#include "config.h"

/// result list. unsorted
config_scsitarget_t config_scsitargets[MAX_SCSITARGETS];

// code to parse one drive entry
config_scsitarget_t *parseScsiTarget(xmlDocPtr doc, xmlNodePtr cur) {
	xmlAttrPtr attr;
	config_scsitarget_t *result = NULL;

//DEBUG("parsePerson\n");
	/* get the TargetID == Disk Index from attribute "id" */
	{
		xmlChar *prop_id = xmlGetProp(cur, "id");
		int id;
		if (!prop_id)
			return NULL; // error
		id = strtol(prop_id, NULL, 0);
		if (id < 0 || id >= MAX_SCSITARGETS)
			return NULL; // error
		result = &(config_scsitargets[id]);
		result->targetId = id;
	}

	/* We don't care what the top level element name is */
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!strcmp(cur->name, "enabled")))
			result->enabled = !strcmp(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), "true");
		if ((!strcmp(cur->name, "deviceType")))
			result->deviceType = strtol(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL,
					0);
		if ((!strcmp(cur->name, "sdSectorStart")))
			result->sectorStart = strtol(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL,
					0);
		if ((!strcmp(cur->name, "scsiSectors")))
			result->sectors = strtol(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL,
					0);
		if ((!strcmp(cur->name, "bytesPerSector")))
			result->bytesPerSector = strtol(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL,
					0);
		if ((!strcmp(cur->name, "sectorsPerTrack")))
			result->sectorsPerTrack = strtol(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL,
					0);
		if ((!strcmp(cur->name, "headsPerCylinder")))
			result->headsPerCylinder = strtol(
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1), NULL,
					0);

		if ((!strcmp(cur->name, "vendor")))
			strcpy(result->vendor,
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
		if ((!strcmp(cur->name, "prodId")))
			strcpy(result->prodId,
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
		if ((!strcmp(cur->name, "revision")))
			strcpy(result->revision,
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));
		if ((!strcmp(cur->name, "serial")))
			strcpy(result->serial,
					xmlNodeListGetString(doc, cur->xmlChildrenNode, 1));

		cur = cur->next;
	}

	return result;
}

/* load list of scsitargets from XML
 *
 * see http://xmlsoft.org/tutorial/apc.html
 *
 * result= 0 = OK, else error
 */
int config_load(char *docname) {

	xmlDocPtr doc;
	xmlNodePtr cur;

	// clear all members of all records
	memset(config_scsitargets, 0, sizeof(config_scsitargets));

	doc = xmlParseFile(docname);

	if (doc == NULL) {
		error("XML Document %s not parsed successfully.", docname);
		return 1;
	}

	/*
	 * Dumping document to stdio or file
	 */
//	xmlSaveFormatFileEnc("-", doc, "UTF-8", 1);
	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		error("empty XML document %s", docname);
		xmlFreeDoc(doc);
		return 2;
	}
	if (xmlStrcmp(cur->name, (const xmlChar *) "SCSI2SD")) {
		error("XML document %s of the wrong type, root node != SCSI2SD",
				docname);
		xmlFreeDoc(doc);
		return 3;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "SCSITarget"))) {
			parseScsiTarget(doc, cur);
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
	return 0;

}

void config_print_scsitarget(FILE *fout, config_scsitarget_t *st) {
	fprintf(fout, "targetId=%d, ", st->targetId);
	fprintf(fout, "enabled=%d, ", st->enabled);
	fprintf(fout, "devicetype=%d,\n", st->deviceType);

	fprintf(fout, "  sectorStart=%d, ", st->sectorStart);
	fprintf(fout, "sectors=%d, ", st->sectors);
	fprintf(fout, "bytesPerSector=%d, ", st->bytesPerSector);
	fprintf(fout, "sectorsPerTrack=%d, ", st->sectorsPerTrack);
	fprintf(fout, "headsPerCylinder=%d,\n", st->headsPerCylinder);

	fprintf(fout, "  vendor=%s, ", st->vendor);
	fprintf(fout, "prodId=%s, ", st->prodId);
	fprintf(fout, "revision=%s, ", st->revision);
	fprintf(fout, "serial=%s\n", st->serial);
}

