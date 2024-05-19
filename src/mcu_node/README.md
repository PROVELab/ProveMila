# MCU Node
The MCU node or main controller unit is responsible for controlling the motor, communicating with the dashboard, and calculating the estimated range.

To improve compile time (multi-factor reduction) on linux, copy .mbedignore into the platformio folder via:
cp .mbedignore ~/.platformio/packages/framework-mbed/platformio/.mbedignore
from home directory

if you get an issue with permissions, do this:
sudo chmod a+rw /dev/ttyACM0

## Specs
Board: NXP mbed LPC1768
Framework: mbedOS + RTOS
Components: Android tablet
