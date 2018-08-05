
A simple spectrumviewer

---------------------------------------------------------------------

![spectrumviewer](/spectrumviewer.png?raw=true)

Since most of the modern devices can handle signals with a substantial 
bandwidth, it is desirable to be able to show such spectra.

spectrumviewer is able to handle the input from wide band devices,
such as the SDRplay, showing a bandwidth of 10 Mhz in the picture.
The program allows "scanning" over a selected band, with a selected
interval and a selected frequency step. The programs shows
at the same time a frequency spectrum and a waterfall.
It - obviously - supports freezing the image.

The current version has three "scopes", the top one showing the
spectrum of the signal, with the second one showing the spectrum
as a waterfall.
The third one, smaller, shows a fraction of the spectrum. The fraction
is determined by a selector. The picture shows a franction of 1/30,
so a resulting width of app 300 KHz.
-----------------------------------------------------------------------

The program can be configured to handle the SDRplay, DABsticks, the AIRspy
and the HACKrf devices.

--------------------------------------------------------------------------

To construct the spectrumviewer adapt the ".pro" file to your needs
For the devices included (by uncommenting the appropriate lines
in the ".pro" file) you need to install the appropriate libraries

The CMakelists.txt was tested as well.


