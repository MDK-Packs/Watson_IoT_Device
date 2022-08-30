# Watson_IoT_Device
CMSIS Software Pack generator adding and merging contributions not yet available in upstream repo.

## Targeted public repository (upstream): 
- Watson Embedded C Client Library: https://github.com/ibm-watson-iot/iot-embeddedc

*Note: this repository in no longer actively maintained.*

This repository provides a bash script to build a CMSIS-Pack. 
It fetches a preconfigured version of the upstream repository, adds a set of
files contained in the directory named 'contributions', and creates a ZIP archive.  
The contributions directory contains the subfolders:
- 'add': containing files for inclusion into the pack that are not (yet) present in the upstream repository
- 'merge': containing files that are present in the public upstream GitHub repository but have been modified to become fit for use in Arm Keil MDK and Arm DS.

The aim is to make the files from the 'add' folder part of the upstream repository via pull-requests. 
Once all files are included in the upstream repository this repository will become obsolete.

## Prerequisites:
- git bash (Windows: https://gitforwindows.org/)
- ZIP archive creation utility (e.g. [7-Zip](https://www.7-zip.org/))
- CMSIS Pack installed in CMSIS_PACK_ROOT (for PackChk utility)
- [Doxygen version 1.8.6](https://sourceforge.net/projects/doxygen/files/rel-1.8.6/)

## Configuration:
In order to build the CMSIS-Pack, you need to run the bash shell script `./gen_pack.sh`
