This repository contains host software (Linux/Windows) for HackRF, a project to
produce a low cost, open source software radio platform.

## How to build the host software on Linux:

### Prerequisites for Linux (Debian/Ubuntu):
`sudo apt-get install build-essential cmake libusb-1.0-0-dev pkg-config libfftw3-dev`

### Build host software on Linux:
```
mkdir host/build
cd host/build
cmake ..
make
sudo make install
sudo ldconfig
```

By default this will attempt to install an udev rule to `/etc/udev/rules.d` to
provide the `usb` or `plugdev` group access to HackRF. If your setup requires
the udev rule to be installed elsewhere you can modify the path with
`-DUDEV_RULES_PATH=/path/to/udev`.

Note: The udev rule is not installed by default for PyBOMBS installs as
they do not ususally get installed with root privileges.

## Clean CMake temporary files/dirs:
```
cd host/build
rm -rf *
```

## How to build host software on Windows:
### Prerequisites for Cygwin, MinGW, or Visual Studio:

* cmake-3.1 or later from http://www.cmake.org/cmake/resources/software.html
* libusb-1.0.18 or later from https://libusb.info/
  * The Visual Studio binaries available on their site are only for Visual Studio 2015.  If you are using a different version of VS you will need to build libusb from source.
* fftw-3.3.5 or later from http://www.fftw.org/install/windows.html
* Install Windows driver for HackRF hardware or use Zadig see http://sourceforge.net/projects/libwdi/files/zadig
  - If you want to use Zadig select HackRF USB device and just install/replace it with WinUSB driver.

If your environment has a package manager, such as Cygwin or MSYS2, you should be able to install these from there.  Otherwise, you can download binaries directly from these sites and copy them to a build dependencies prefix. DLLs go in the bin folder, headers in include, .dll.a and .lib files in the lib folder, like normal.

>**Note for Windows build:**
 You shall always execute hackrf-tools from Windows command shell and not from Cygwin or MinGW shell because on Cygwin/MinGW
 Ctrl C is not managed correctly and especially for hackrf_transfer the Ctrl C(abort) will not stop correctly and will corrupt the file.

### For Cygwin:
```
mkdir host/build
cd host/build
cmake ../ -G "Unix Makefiles" -DCMAKE_LEGACY_CYGWIN_WIN32=1 -DLIBUSB_INCLUDE_DIR=/usr/local/include/libusb-1.0/
make
make install
```

### For MinGW:

If you downloaded the fftw3 binaries above, you have to turn that fftw3 DLL into something MinGW can link to by running these commands in your prefix bin folder:
```
C:\your\build\prefix\bin> dlltool -d libfftw3f-3.def -l libfftw3f-3.dll.a -D libfftw3f-3.dll
C:\your\build\prefix\bin> dlltool -d libfftw3-3.def -l libfftw3-3.dll.a -D libfftw3-3.dll
```

Then move the generated .dll.a files to your prefix lib folder.

Now you can build from a Windows command prompt:
```
mkdir host/build
cd host/build
cmake ../ -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/your/build/prefix -DCMAKE_INSTALL_PREFIX=C:/your/install/prefix
mingw32-make
mingw32-make install
```

### For Visual Studio 2015 x64

Similarly to the MinGW instructions, create fftw3 library definitions for MSVC to link to.  You may have to run these from a Visual Studio command prompt:
```
C:\your\build\prefix\bin> lib /machine:x64 /def:libfftw3f-3.def
C:\your\build\prefix\bin> lib /machine:x64 /def:libfftw3-3.def
```
Then move the generated .lib files to your prefix lib folder.  Also rename them to get rid of the "lib" prefix, e.g. `fftw3-3.lib`.

```
c:\hackrf\host\build> cmake ../ -G "Visual Studio 14 2015 Win64" -DCMAKE_PREFIX_PATH=C:/your/build/prefix -DCMAKE_INSTALL_PREFIX=C:/your/install/prefix
```

CMake will produce a solution file named `HackRF.sln` and a series of
project files which can be built with msbuild as follows:
`c:\hackrf\host\build> msbuild HackRF.sln`

## How to build host the software on FreeBSD
You can use the binary package:
`# pkg install hackrf`

You can build and install from ports:
```
# cd /usr/ports/comms/hackrf
# make install
```

principal author: Michael Ossmann <mike@ossmann.com>

http://greatscottgadgets.com/hackrf/
