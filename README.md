# PROVE Lab - Mila (Electric Endurance Vehicle)
Mila is an electric sports car with the goal of going 1000 miles on a single charge. This is the GitHub repository for the Low Voltage system. It's built on [PlatformIO](https://platformio.org/).

## Project Structure
The `src` folder contains the source code for all the embedded components of the system. Inside the `src` folder, there are folders for each node of the system and a `common` folder for shared code and drivers. See the READMEs within each folder for further information about each node.

## "Linting"
In VSCode, under C/C++ extension format style, place this: 
{ BasedOnStyle: Google, UseTab: Never, IndentWidth: 4, AccessModifierOffset: -4, ColumnLimit: 80}
for our linting style

## Optimizations
To improve compile time (multi-factor reduction) on linux, copy .mbedignore into the platformio folder via:
cp .mbedignore ~/.platformio/packages/framework-mbed/platformio/.mbedignore
from home directory
When writing code,
peephole optimize what you can (or ask Shynn to do so).

## Permission Issues
if you get an issue with permissions, do this:
sudo chmod a+rw /dev/ttyACM0
where ttyACM0 represents the specific port which is not
being uploaded to
