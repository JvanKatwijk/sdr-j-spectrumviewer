
A simple spectrumviewer

---------------------------------------------------------------------

![spectrumviewer](/spectrumviewer.png?raw=true)

Since most of the modern devices can handle signals with a substantial 
bandwidth, it is desirable to be able to show such spectra.

sdr-j-spectrumviewer is able to handle the input from wide band devices,
such as the SDRplay, showing a bandwidth of 8 Mhz in the picture,
to a soundcard with a bandwidth of only 48k.
The program allows "scanning" over a selected band, with a selected
interval and a selected frequency step. The programs shows
at the same time a frequency spectrum and a waterfall.
It - obviously - supports freezing the image.

-----------------------------------------------------------------------

The program can be configured to handle the SDRplay, DABsticks, the AIRspy
and the HACKrf devices.

--------------------------------------------------------------------------

To construct the spectrumviewer adapt the ".pro" file to your needs
For the devices included (by uncommenting the appropriate lines
in the ".pro" file) you need to install the appropriate libraries

The CMakelists.txt was tested as well.


