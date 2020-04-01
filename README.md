

---------------------------------------------------------------------
A simple spectrumviewer
---------------------------------------------------------------------

![spectrumviewer](/spectrumviewer.png?raw=true)

Since most of modern  sdr devices can handle signals with a substantial 
bandwidth, it is desirable to be able to show such spectra.

*spectrumviewer* is able to handle the input from wide band devices,
such as the SDRplay, hackrf devices and limeSDR, each
showing a bandwidth of 10 Mhz in the picture.

The program allows "scanning" over a selected band, with a selected
interval and a selected frequency step. The programs shows
at the same time a frequency spectrum and a waterfall.

The current version has three "scopes", the top one showing the
spectrum of the signal, with the second one showing the spectrum
as a waterfall.

The third one, smaller, allows you to look in greater detail to
a segment of the spectrum. The amount of detail is determined by
the decimation setting, a range between 5 and 100.

Added is a view on the signal - as presented to the detailed viewer -
as appearing in the time domain.

The current version is a redesign and the GUI is modernized.

---------------------------------------------------------------------------
Windows
-----------------------------------------------------------------------------

Soon an installer for Windows will be created

--------------------------------------------------------------------------------
Linux
------------------------------------------------------------------------

To construct the spectrumviewer adapt the ".pro" file to your needs
For the devices included (by uncommenting the appropriate lines
in the ".pro" file) you need to install the appropriate libraries

-----------------------------------------------------------------------
Supported devices
-----------------------------------------------------------------------

The program can be configured to handle the SDRplay, DABsticks, the AIRspy,
the HACKrf and the limeSDR.

The software will - on startup - perform a check to see which device
is available, depending on the configured devices.

The current implementation supports dynamically selecting the
width of the signal for the SDRplay, the AIRspy and the limeSDR,
the bandwidth for the Hackrf is set to 10 Mhz, 

The software does support the full frequency range for the Hackrf device,
i.e. up to 6000 Mhz, while the other devices are limited to app 2 GHz.

--------------------------------------------------------------------------

