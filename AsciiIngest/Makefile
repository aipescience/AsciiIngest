# Copyright (C) 2009, Steffen Knollmann & Adrian Partl

include ../Makefile.config

.PHONY: all clean

# Define the sources
sources = AsciiHeaderReader.cpp\
	  AsciiReader.cpp\
	  AsciiSchemaMapper.cpp\
	  AsciiDataObjDesc.cpp\
	  DataObjDesc.cpp \
          HeaderReader.cpp \
	  asciiingest_error.cpp\
	  dbingestor_error.cpp\
          Reader.cpp \
          Schema.cpp \
          SchemaDataMapGenerator.cpp \
          SchemaItem.cpp\
	  DType.cpp\
	  main.cpp

# Include additional rules
include ../Makefile.rules

# Catch empty call
all:
	$(MAKE) AsciiIngest

# Cleaning up
clean:
	rm -f AsciiIngest  $(sources:.cpp=.o) $(sources:.cpp=.d)

# How to build the main programs
AsciiIngest: $(sources:.cpp=.o) 
	$(CC) $(CFLAGS) -o AsciiIngest $(sources:.cpp=.o) \
	                          -lm

# Now include all objects
-include $(sources:.cpp=.d)

# External dependencies
#libadts/libadts.a:
#	$(MAKE) -C libadts
#
#libutil/libutil.a:
#	$(MAKE) -C libutil
#
#libio/libio.a:
#	$(MAKE) -C libio
