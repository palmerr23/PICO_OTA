# PI PICO W OTA and File System Management

The Arduino V2 IDE does not yet include a file upload plugin for the RP2040.

## This program provides a web-based LittleFS interface for
* OTA (BIN files)
* File uploding ('Blank' file systems are formatted automatically on mounting.)
* FS directory listing
* Text-based file editing
* File deletion 

## Use
* It may be accessed via from a browser by its IP address (output via the USB serial interface on booting) or at http://PICO-OTA.local
* OTA requires standard BIN format files.
* The program must be compiled with a filesystem of sufficient size to hold the uploaded OTA binary file.
* Pre-existing 'firmware.bin' and 'otacommand.bin' files are automatically deleted on each web page reload.
* The 'firmware.bin' and 'otacommand.bin' files will be in the flash filesystem after rebooting with the new uploaded program, and must be deleted if the filesystem space is required.

## Limitations:
* The program currently supports only LittleFS on the onboard flash memory.

* WiFi credentials must be compiled in.
* The code has only been tested in a Chrome desktop browser on Windows. While it should function correclty in Mozilla-style browsers across all desktop OS platforms, functionality on mobile and other desktop browsers is untested.
* The text editor uses files from the internet. The host computer must be conntected to the internet for this function to work.

## To Do
* Add SDFS (SD card) support.
