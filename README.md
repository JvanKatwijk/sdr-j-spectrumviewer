
To construct the spectrumviewer adapt the ".pro" file to your needs
For the devices included (by uncommenting the appropriate lines
in the ".pro" file) you need to install the appropriate libraries

The CMakelists.txt was tested as well.


#######################################################################
Note on DABstick software

I had (far) better results using an RT820T based dabstick with generating
a library from 
http://www.rtl-sdr.com/new-experimental-r820t-rtl-sdr-driver-tunes-13-mhz-lower/
###########################################################################

One may also look at the work of Leif for Linrad

http://www.sm5bsz.com/linuxdsp/hware/rtlsdr/rtlsdr.htm

There an alternative rtl-sdr library is available
##########################################################################

The boundaries for "legalFrequency" in "input/dabstick/dabstick.cpp"
were set to accomodate the extended range as mentioned above.
In my settings, real signal starts at app 13 Mhz

