# Arduino Shield Programmer for ROM-el 2364
## Updates / Changes September 2023
* Added schematic for Arduino Shield
    * Test on: Duemilanove, Uno R3, and Mega R3
* Updated Arduino code and Python script data types to extend them to 32-bits for AT49F001 which has 17-bit addressing for the full range.
    * Added basic time duration to displayed output
    * Pin header to attach address
* Tested on Windows and Linux with Python 3
    * Comm port address is now a variable
* [Shared on OSH park](https://oshpark.com/shared_projects/amO60ueQ)


# ROM-el 2364 Programmer

This is an Arduino sketch and companion Python application to read from and write to ROM-el 2364 devices that (as of June 2018) ship with the AT49F512 EEPROM chip. It should, however ,also work with the AT49F001 EEPROM that was originally shipped on the ROM-el 2364.

The Python script was written and tested on a Linux system, and may need some tweaks to run in Windows environments, such as changing the serial port in the init_arduino() routine from /dev/ttyACM0 to, i.e. COM1.
