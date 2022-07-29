The program cycles through all files in its folder and finds the Milkdrop presets with one of the lines:

PSVERSION=4
PSVERSION_WARP=4
PSVERSION_COMP=4

For those lines the pixel shader version will be changed from 4 to 3.

This is a Java program and you need a Java runtime environment to execute.
Somehow the jar file does not return any information to the console if not executed through the batch file - the preset files will be changed then anyway.
If the batch file doesn't point to your JRE directory edit the corresponding line first.