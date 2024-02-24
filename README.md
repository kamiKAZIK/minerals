# minerals

From Arduino to a Microcontroller on a Breadboard

Unless you choose to use the minimal configuration described at the end of this tutorial, you'll need four components (besides the Arduino, ATmega328P, and breadboard):
 - a 16 MHz crystal,
 - a 10k resistor, and
 - two 18 to 22 picofarad (ceramic) capacitors.

Burning the Bootloader

If you have a new ATmega328P (or ATmega168), you'll need to burn the bootloader onto it. You can do this using an Arduino board as an in-system program (ISP). If the microcontroller already has the bootloader on it (e.g. because you took it out of an Arduino board or ordered an already-bootloaded ATmega), you can skip this section.

To burn the bootloader, follow these steps:
 - Upload the ArduinoISP sketch onto your Arduino board. (You'll need to select the board and serial port from the Tools menu that correspond to your board.)
 -  Wire up the Arduino board and microcontroller as shown in the diagram to the right.
 - Select "Arduino Duemilanove or Nano w/ ATmega328" from the Tools > Board menu. (Or "ATmega328 on a breadboard (8 MHz internal clock)" if using the minimal configuration described below.)
 - Select "Arduino as ISP" from Tools > Programmer
 - Run Tools > Burn Bootloader

You should only need to burn the bootloader once. After you've done so, you can remove the jumper wires connected to pins 10, 11, 12, and 13 of the Arduino board.
