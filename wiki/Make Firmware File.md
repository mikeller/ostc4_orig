# How to Make an Uploadable Firmware File for the OSTC4 #

Generated BIN files from the SMT32 Workbench need modification in order to be installed via the OSTC Companion onto the OSTC4. 

## Build Command Line Tools ##

These can be found in /ostc4pack (For Windows, CPP sources available)

## Run the Tools Manually ##

1. Copy generated BIN files (From the Release or debug directory) into /ostc4pack. Or: Modify .bat files to point to your Release or debug directory.
2. run step1_create_OSTC_RTE_upload.bat and/or step1_create_OSTC4_firmware_upload_bin.bat (Depending on which part you made changes)
3. run step2_create_OSTC4update_bin.bat to create final OSTC4update_xxxxxx.bin, with xxxxxx being the current date in YYMMDD format. This will add important checksums required for the OSTC4's bootloader to accept the file.

## Upload this file with the OSTC Companion software to the OSTC4 ##

See http://forum.heinrichsweikamp.com/read.php?7,19186 on how to get and use the "OSTC companion"