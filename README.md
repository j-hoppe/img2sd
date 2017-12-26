# img2sd
moves SCSI disk images from and to a SCSI2SD SDcard.

## About SCSI2SD
The SCSI2SD hardware implements up to 4 SCSI hard disks on a single SDcard, see
[www.codesrc.com](http://www.codesrc.com/mediawiki/index.php?title=SCSI2SD)

This allows to operate vintage computer hardware without physical SCSI drives.

The partition layout of a SDcard is defined with the tool "`scsi2sd-util`", and kept as an XML file.

"img2sd" reads the XML config file, then allows to read/write/verify the SDcard partition against binary file images.

This could also be done with "`dd`" command, but then block offset and sizes must be calculated manually.


## Compile
Compile tested under Ubuntu. The "libxml" package is needed.
Install with

    sudo apt-get install libxml2-dev

See `makefile` for more info.

##  Operation
The build-in help should say all:

```
joerg@vmubuprog:~/progs/RetroCmp/img2sd$ ./img2sd
img2sd - moves SimH disk images from and to SCSI2SD SDcard
   version: Dec 26 2017 08:49:33
   Contact: j_hoppe@t-online.de, retrocmp.com

   For SCSI2SD doc and downloads see "http://www.codesrc.com/mediawiki/index.php?title=SCSI2SD"

Command line summary:

img2sd  --help --verbose --device <device_filename> --xml <config_filename>
          --read <target_id> <image_file> --write <target_id> <image_file>
          --compare <target_id> <image_file>
          --writecompare <target_id> <image_file>

-?  | --help
          Print help
-v  | --verbose
          Verbose output
-d  | --device <device_filename>
          Raw SDCard device, without "/dev".
          To check: plug in SDcard, then "dmesg | tail"
          Simple example:  -d sdb
              Use "/dev/sdb" as interface to SDcard.
-x  | --xml <config_filename>
          Path to mandatory SCSI2SD geometry config file (XML)
          Simple example:  -x 4xRD54_rev471.xml
              The XML file must be generated with "scsi2sd-util".
-r  | --read <target_id> <image_file>
          Read disk image from SDcard partition.
          Simple example:  -r 3,rsxdata.img
              Read partition with SCSI ID #3 and save it as file "rsxdata.img".
-w  | --write <target_id> <image_file>
          Write disk image into SDcard partition. Size must fit!
          Simple example:  -w 0,rt1157.rd54
              Copy the disk image file "rt1157.rd54" onto drive #0 partition
              Offset and size on SDcard is taken from XML config file.
-c  | --compare <target_id> <image_file>
          Compare disk image file with SDcard partition.
-wc | --writecompare <target_id> <image_file>
          First write, then compare

Option names are case insensitive.
```
