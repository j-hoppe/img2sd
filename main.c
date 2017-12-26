/* main.c: moves SimH disk images to SCSI2SD SDcard

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

#define VERSION	"v1.0"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "error.h"
#include "utils.h"
#include "getopt2.h"
#include "config.h"

// command line args
getopt_t getopt_parser;

int arg_menu_linewidth = 80;

int opt_verbose = 0;
char opt_device[PATH_MAX]; // path to SDcard device
char opt_config[PATH_MAX]; // path of SCSI2SD config XML

static void banner() {
	fprintf(stdout,
			"img2sd - moves SimH disk images from and to SCSI2SD SDcard\n");
	fprintf(stdout, "   version: "__DATE__ " " __TIME__ "\n");
}

/*
 * help()
 */
static void help() {
	fprintf(stdout, "   Contact: j_hoppe@t-online.de, retrocmp.com\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "   For SCSI2SD doc and downloads see \"http://www.codesrc.com/mediawiki/index.php?title=SCSI2SD\"\n") ;
	fprintf(stdout, "\n");
	fprintf(stdout, "Command line summary:\n\n");
	// getop must be initialized to print the syntax
	getopt_help(&getopt_parser, stdout, arg_menu_linewidth, 10, "img2sd");
	exit(1);
}

// show error for one option
static void commandline_error() {
	fprintf(stdout, "Error while parsing commandline:\n");
	fprintf(stdout, "  %s\n", getopt_parser.curerrortext);
	exit(1);
}

// parameter wrong for currently parsed option
static void commandline_option_error(char *errtext, ...) {
	va_list args;
	fprintf(stdout, "Error while parsing command line option:\n");
	if (errtext) {
		va_start(args, errtext);
		vfprintf(stderr, errtext, args);
		fprintf(stderr, "\nSyntax:  ");
		va_end(args);
	} else
		fprintf(stderr, "  %s\nSyntax:  ", getopt_parser.curerrortext);
	getopt_help_option(&getopt_parser, stdout, 96, 10);
	exit(1);
}

/* core function: read and write sdcard
 * only the partiiton of card file is read, which is defined
 * by the SCSI target id and geometry data in "config"
 */
#define BUFFER_SIZE	(1024 *1024) // copy in chunks of 1M

static void sdcard_read(int target_id, char *sdcard_filename,
		char *image_filename) {
	int fd_card, fd_img;
	config_scsitarget_t *scsitarget;
	int64_t offset, size;
	int64_t bytesToWrite;
	int64_t bytesWritten;

	if (target_id < 0 || target_id > MAX_SCSITARGETS)
		error("Invalid target id %d", target_id);
	scsitarget = &config_scsitargets[target_id];
	if (!scsitarget->enabled)
		error("Target id %d not enabled", target_id);

	offset = scsitarget->bytesPerSector * (int64_t) (scsitarget->sectorStart);
	size = scsitarget->bytesPerSector * (int64_t) (scsitarget->sectors);

	if (opt_verbose) {
		info("Reading SCSI ID %d on SDcard \"%s\" to file \"%s\".", target_id,
				sdcard_filename, image_filename);
		info(
				"SDcard offset = %ld bytes = %ld sectors, size = %ld bytes = %ld sectors.",
				offset, offset / scsitarget->bytesPerSector, size,
				size / scsitarget->bytesPerSector);
	}

	fd_card = open(sdcard_filename, O_RDWR); // must exist
	if (fd_card < 0)
		error("Can not open SDcard file \"%s\" for read (sudo?)",
				sdcard_filename);
	fd_img = open(image_filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd_img < 0)
		error("Can not open image file \"%s\" for write", image_filename);

	if (lseek(fd_card, offset, SEEK_SET) < 0)
		error("SDcard seek failed with errno = %d", errno);

	// copy loop
	bytesToWrite = size;
	bytesWritten = 0;
	while (bytesToWrite > 0) {
		char buffer[BUFFER_SIZE];
		int block_size;
		if (bytesToWrite >= BUFFER_SIZE)
			block_size = BUFFER_SIZE;
		else
			block_size = bytesToWrite; // end of stream
		bytesToWrite -= block_size;

		read(fd_card, buffer, block_size);
		write(fd_img, buffer, block_size);
		bytesWritten += block_size;
		if (opt_verbose) { // moving percent indicator, no \n
			printf("\rRead completed %3ld%% ", (bytesWritten * 100) / size);
			fflush(stdout);
		}
	}
	close(fd_card);
	close(fd_img);
	if (opt_verbose)
		printf("\n");
}

static void sdcard_write(int target_id, char *sdcard_filename,
		char *image_filename) {
	int fd_card, fd_img;
	struct stat statbuf;
	config_scsitarget_t *scsitarget;
	int64_t offset, size;
	int64_t bytesToWrite;
	int64_t bytesWritten;

	if (target_id < 0 || target_id > MAX_SCSITARGETS)
		error("Invalid target id %d", target_id);
	scsitarget = &config_scsitargets[target_id];
	if (!scsitarget->enabled)
		error("Target id %d not enabled", target_id);

	offset = scsitarget->bytesPerSector * (int64_t) (scsitarget->sectorStart);
	size = scsitarget->bytesPerSector * (int64_t) (scsitarget->sectors);

	// file size
	if (stat(image_filename, &statbuf) < 0)
		error("Can not open file \"%s\"", image_filename);
	bytesToWrite = statbuf.st_size;

	if (opt_verbose) {
		info("Writing SCSI ID %d on SDcard \"%s\" from file \"%s\".", target_id,
				sdcard_filename, image_filename);
		info(
				"SDcard offset = %ld bytes = %ld sectors, size = %ld bytes = %ld sectors.",
				offset, offset / scsitarget->bytesPerSector, size,
				size / scsitarget->bytesPerSector);
	}

	if (bytesToWrite % scsitarget->bytesPerSector)
		error("Size of file \"%s\" is %ld, not a multiple of sector size %d",
				image_filename, bytesToWrite, scsitarget->bytesPerSector);
	if (bytesToWrite > size)
		error(
				"Image file too large: Size of file \"%s\" is %ld, size of SCSI ID %d is %ld",
				image_filename, bytesToWrite, target_id, size);
	if (bytesToWrite < size)
		warning(
				"Image file too small: Size of file \"%s\" is %ld, size of SCSI ID %d is %ld",
				image_filename, bytesToWrite, target_id, size);

	fd_card = open(sdcard_filename, O_WRONLY | O_TRUNC); // must exist
	if (fd_card < 0)
		error("Can not open sdcard file \"%s\" for write (sudo?)",
				sdcard_filename);
	fd_img = open(image_filename, O_RDWR);
	if (fd_img < 0)
		error("Can not open image file \"%s\" for read", image_filename);

	if (lseek(fd_card, offset, SEEK_SET) < 0)
		error("SDcard seek failed with errno = %d", errno);

	// copy loop
	bytesToWrite = size;
	bytesWritten = 0;
	while (bytesToWrite > 0) {
		char buffer[BUFFER_SIZE];
		int block_size;
		if (bytesToWrite >= BUFFER_SIZE)
			block_size = BUFFER_SIZE;
		else
			block_size = bytesToWrite; // end of stream
		bytesToWrite -= block_size;

		read(fd_img, buffer, block_size);
		write(fd_card, buffer, block_size);
		bytesWritten += block_size;
		if (opt_verbose) { // moving percent indicator, no \n
			printf("\rWrite completed %3ld%% ", (bytesWritten * 100) / size);
			fflush(stdout);
		}
	}
	close(fd_card);
	close(fd_img);
	if (opt_verbose)
		printf("\n");
}

static void sdcard_verify(int target_id, char *sdcard_filename,
		char *image_filename, int shortinfo) {
	int fd_card, fd_img;
	struct stat statbuf;
	config_scsitarget_t *scsitarget;
	int64_t offset, size;
	int64_t bytesToRead;
	int64_t bytesRead;

	if (target_id < 0 || target_id > MAX_SCSITARGETS)
		error("Invalid target id %d", target_id);
	scsitarget = &config_scsitargets[target_id];
	if (!scsitarget->enabled)
		error("Target id %d not enabled", target_id);

	offset = scsitarget->bytesPerSector * (int64_t) (scsitarget->sectorStart);
	size = scsitarget->bytesPerSector * (int64_t) (scsitarget->sectors);

	// file size
	if (stat(image_filename, &statbuf) < 0)
		error("Cannot open image file \"%s\"", image_filename);
	bytesToRead = statbuf.st_size;

	if (opt_verbose && !shortinfo) {
		info("Verifying SCSI ID %d on SDcard \"%s\" with file \"%s\".",
				target_id, sdcard_filename, image_filename);
		info(
				"SDcard offset = %ld bytes = %ld sectors, size = %ld bytes = %ld sectors.",
				offset, offset / scsitarget->bytesPerSector, size,
				size / scsitarget->bytesPerSector);
	}

	if (bytesToRead % scsitarget->bytesPerSector)
		error("Size of file \"%s\" is %ld, not a multiple of sector size %d",
				image_filename, bytesToRead, scsitarget->bytesPerSector);
	if (bytesToRead > size)
		error(
				"Image file too large: Size of file \"%s\" is %ld, size of SCSI ID %d is %ld",
				image_filename, bytesToRead, target_id, size);
	if (bytesToRead < size && !shortinfo)
		warning(
				"Image file is smaller: Size of file \"%s\" is %ld, size of SCSI ID %d is %ld",
				image_filename, bytesToRead, target_id, size);

	fd_card = open(sdcard_filename, O_RDWR);
	if (fd_card < 0)
		error("Can not open sdcard file \"%s\" for read (sudo?)",
				sdcard_filename);
	fd_img = open(image_filename, O_RDWR);
	if (fd_img < 0)
		error("Can not open image file \"%s\" for read", image_filename);

	if (lseek(fd_card, offset, SEEK_SET) < 0)
		error("SDcard seek failed with errno = %d", errno);

	// verify loop. bytesToRead minimum of file and SDcard size
	bytesToRead = size;
	bytesRead = 0;
	while (bytesToRead > 0) {
		char buffer_card[BUFFER_SIZE];
		char buffer_img[BUFFER_SIZE];
		int block_size;
		if (bytesToRead >= BUFFER_SIZE)
			block_size = BUFFER_SIZE;
		else
			block_size = bytesToRead; // end of stream

		read(fd_img, buffer_img, block_size);
		read(fd_card, buffer_card, block_size);
		if (memcmp(buffer_card, buffer_img, block_size)) {
			error("Data mismatch between bytes %ld and %ld", bytesRead,
					bytesRead + block_size);
		}
		bytesToRead -= block_size;
		bytesRead += block_size;
		if (opt_verbose) { // moving percent indicator, no \n
			printf("\rVerify completed %3ld%% ", (bytesRead * 100) / size);
			fflush(stdout);
		}
	}

	close(fd_card);
	close(fd_img);
	if (opt_verbose)
		printf("\n");
}

/*
 * read command line parameters into global vars
 * result: 0 = OK, 1 = error
 */
static void parse_commandline(int argc, char **argv) {
	int res;

	// define commandline syntax
	getopt_init(&getopt_parser, /*ignore_case*/1);

	// !!!1 Do not define any defaults... else these will be set very time!!!

	getopt_def(&getopt_parser, "?", "help", NULL, NULL, NULL, "Print help",
	NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "v", "verbose", NULL, NULL, NULL,
			"Verbose output",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "d", "device", "device_filename", NULL, NULL,
			"Raw SDCard device, without \"/dev\".\n"
			"To check: plug in SDcard, then \"dmesg | tail\"",
			"sdb", "Use \"/dev/sdb\" as interface to SDcard.", NULL, NULL);
	getopt_def(&getopt_parser, "x", "xml", "config_filename", NULL, NULL,
			"Path to mandatory SCSI2SD geometry config file (XML)",
			"4xRD54_rev471.xml", "The XML file must be generated with \"scsi2sd-util\".\n", NULL, NULL);
	getopt_def(&getopt_parser, "r", "read", "target_id,image_file", NULL, NULL,
			"Read disk image from SDcard partition.",
			"3,rsxdata.img",
			"Read partition with SCSI ID #3 and save it as file \"rsxdata.img\".",
			NULL, NULL);
	getopt_def(&getopt_parser, "w", "write", "target_id,image_file", NULL, NULL,
			"Write disk image into SDcard partition. Size must fit!",
			"0,rt1157.rd54",
			"Copy the disk image file \"rt1157.rd54\" onto drive #0 partition\n"
					"Offset and size on SDcard is taken from XML config file.",
			NULL, NULL);
	getopt_def(&getopt_parser, "c", "compare", "target_id,image_file", NULL,
			NULL, "Compare disk image file with SDcard partition.",
			NULL, NULL, NULL, NULL);
	getopt_def(&getopt_parser, "wc", "writecompare", "target_id,image_file",
			NULL, NULL, "First write, then compare", NULL, NULL, NULL, NULL);

	if (argc < 2)
		help(); // at least 1 required

	res = getopt_first(&getopt_parser, argc, argv);
	while (res > 0) {
		if (getopt_isoption(&getopt_parser, "help")) {
			help();
		} else if (getopt_isoption(&getopt_parser, "verbose")) {
			opt_verbose = 1;
		} else if (getopt_isoption(&getopt_parser, "device")) {
			char buffer[80];
			if (getopt_arg_s(&getopt_parser, "device_filename", buffer,
					sizeof(buffer)) < 0)
				commandline_option_error(NULL);
			strcpy(opt_device, "/dev/");
			strcat(opt_device, buffer);
			if (access(opt_device, F_OK) == -1)
				commandline_option_error("SDcard device does not exist");
		} else if (getopt_isoption(&getopt_parser, "xml")) {
			int id;
			if (getopt_arg_s(&getopt_parser, "config_filename", opt_config,
					sizeof(opt_config)) < 0)
				commandline_option_error(NULL);
			if (access(opt_config, R_OK) == -1)
				commandline_option_error("config file can not be read");
			if (config_load(opt_config)) {
				error("XML file error!\n");
			}
			if (opt_verbose) {
				info("SCSI target disks read from \"%s\":", opt_config);
				for (id = 0; id < MAX_SCSITARGETS; id++) {
					config_scsitarget_t *scsitarget = &config_scsitargets[id];
					if (scsitarget->enabled)
						config_print_scsitarget(stdout, scsitarget);
				}
			}

		} else if (getopt_isoption(&getopt_parser, "read")) {
			int target_id;
			char image_file[PATH_MAX];
			if (getopt_arg_i(&getopt_parser, "target_id", &target_id) < 0)
				commandline_option_error(NULL);
			if (getopt_arg_s(&getopt_parser, "image_file", image_file,
					sizeof(image_file)) < 0)
				commandline_option_error(NULL);
			sdcard_read(target_id, opt_device, image_file);
		} else if (getopt_isoption(&getopt_parser, "write")) {
			int target_id;
			char image_file[PATH_MAX];
			if (getopt_arg_i(&getopt_parser, "target_id", &target_id) < 0)
				commandline_option_error(NULL);
			if (getopt_arg_s(&getopt_parser, "image_file", image_file,
					sizeof(image_file)) < 0)
				commandline_option_error(NULL);
			sdcard_write(target_id, opt_device, image_file);
		} else if (getopt_isoption(&getopt_parser, "compare")) {
			int target_id;
			char image_file[PATH_MAX];
			if (getopt_arg_i(&getopt_parser, "target_id", &target_id) < 0)
				commandline_option_error(NULL);
			if (getopt_arg_s(&getopt_parser, "image_file", image_file,
					sizeof(image_file)) < 0)
				commandline_option_error(NULL);
			sdcard_verify(target_id, opt_device, image_file, 0);
		} else if (getopt_isoption(&getopt_parser, "writecompare")) {
			int target_id;
			char image_file[PATH_MAX];
			if (getopt_arg_i(&getopt_parser, "target_id", &target_id) < 0)
				commandline_option_error(NULL);
			if (getopt_arg_s(&getopt_parser, "image_file", image_file,
					sizeof(image_file)) < 0)
				commandline_option_error(NULL);
			sdcard_write(target_id, opt_device, image_file);
			sdcard_verify(target_id, opt_device, image_file, 1); // fewer output
		}
		res = getopt_next(&getopt_parser);
	}
	if (res == GETOPT_STATUS_MINARGCOUNT || res == GETOPT_STATUS_MAXARGCOUNT)
		// known option, but wrong number of arguments
		commandline_option_error("Illegal argument count");
	else if (res < 0)
		commandline_error();
}

int main(int argc, char *argv[]) {
	ferr = stderr;
	banner();
	parse_commandline(argc, argv);
	// returns only if everything is OK
	// Std options already executed

	return 0;
}
