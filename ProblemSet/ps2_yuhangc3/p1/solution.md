### P1 Solution

1. to separate screen on VGA:

At the start of each frame, circuitry scan display memory for data starting at the address specified by the start address registers. And the scan line that matches the split screen scan line is not part of the split screen; the split screen starts on the following scan lines.

To TURN ON - Set the split screen start scan line to a READABLE register, set start address before split screen. 

To TURN OFF – Set split screen start scan line to a value greater than or equal to the last scan line displayed.


2. to change color palette on VGA:
(-> means output/write to)

palette entry values -> PEL Address Write Mode Register;

component values -> PEL Data Register in order RGB;

PEL Address Write Mode Register will increment automatically;

component values of the palette entry -> the Data Register;

