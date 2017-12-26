#
# CC Command
#

# You need to install XML support: libxml ! 
# 1. http://xmlsoft.org
# 2. sudo apt-get install libxml2-dev
# 3. get -cflags with "xml2-config --cflags"
#    =>  -I/usr/include/libxml2
# 4. get libary path with "xml2-config --libs"
#    =>  -lxml2

# compiler flags and libraries
CC_DBG_FLAGS = -ggdb3 -O0 
# CC_DBG_FLAGS = -ggdb3 -O0 -Wall -Wextra
CCDEFS =-I/usr/include/libxml2
# CCDEFS =-DLIBXML_OUTPUT_ENABLED -DLIBXML_TREE_ENABLED -I/usr/include/libxml2
LDFLAGS=-lxml2

PROG=img2sd


#########################################################
SOURCES.h = \
	error.h	\
	config.h	\
	utils.h	\
    getopt2.h

SOURCES.c = \
	main.c	\
	error.c	\
	config.c	\
	utils.c	\
	getopt2.c

OBJECTS = $(SOURCES.c:%.c=%.o)

#
# Build everything
#
all:    img2sd

clean:
	pwd
	rm -f a.out core $(OBJDIR)/*.lst $(PROG) $(OBJDIR)/$(PROG) $(OBJECTS)


img2sd:	$(SOURCES.c) $(SOURCES.h)
	$(CC) $^ -o $@ $(CC_DBG_FLAGS) $(CCDEFS) $(LDFLAGS)
	file $@

