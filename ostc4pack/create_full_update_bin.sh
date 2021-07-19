#!/bin/bash

#
# path and file name settings
#

# the build products are here
BUILD_PATH="$HOME/git/ostc4/projects"

# Debug or Release build
BUILD_TYPE="Release"

# build project names
CPU1_DISCOVERY="ostc4_discovery"
CPU1_FONTPACK="ostc4_fontpack"
CPU2_RTE="ostc4_rte"


#
# End of path and file name settings
#

#
# Copy the bin files to pack. Build them seperately
#
cp $BUILD_PATH/$CPU1_DISCOVERY/$BUILD_TYPE/$CPU1_DISCOVERY.bin .
cp $BUILD_PATH/$CPU1_FONTPACK/$BUILD_TYPE/$CPU1_FONTPACK.bin .
cp $BUILD_PATH/$CPU2_RTE/$BUILD_TYPE/$CPU2_RTE.bin .

#
# OSTC4pack_V4 all
#
./src/OSTC4pack_V4 1 $CPU1_DISCOVERY.bin
./src/OSTC4pack_V4 2 $CPU1_FONTPACK.bin
./src/OSTC4pack_V4 0 $CPU2_RTE.bin

#
# Final pack
#
./src/checksum_final_add_fletcher ${CPU1_DISCOVERY}_upload.bin \
				${CPU1_FONTPACK}_upload.bin \
				${CPU2_RTE}_upload.bin
