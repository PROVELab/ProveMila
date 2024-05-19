# PROVE Lab - Mila (Electric Endurance Vehicle)
Mila is an electric sports car with the goal of going 1000 miles on a single charge. This is the GitHub repository for the Low Voltage system. It's built on [PlatformIO](https://platformio.org/).

## Project Structure
The `src` folder contains the source code for all the embedded components of the system. Inside the `src` folder, there are folders for each node of the system and a `common` folder for shared code and drivers. See the READMEs within each folder for further information about each node.

## "Linting"
In VSCode, under C/C++ extension format style, place this: 
{ BasedOnStyle: Google, UseTab: Never, IndentWidth: 4, AccessModifierOffset: -4, ColumnLimit: 80}
for our linting style