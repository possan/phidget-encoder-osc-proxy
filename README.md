# Simple phidget encoder to OSC proxy

## Usage

Run `./proxy 3000 528905 528391` to listen for changes to two encoders with serial numbers `528905` and `528391`, and send updates over OSC port `3000`, the OSC payload is two integers, absolute value and relative value, the address is `/phidgets/<serial number>/encoder/<encoder index>` - Exit with Ctrl-C

## Building 

OSX:
* Install Phidget21 libraries (See [these docs](https://www.phidgets.com/docs21/Language_-_C))
* Run `make`
* Run `./proxy`

Windows:
* Download Phidget21 libraries (See [these docs](https://www.phidgets.com/docs21/Language_-_C))
* And put in `lib/` folder.
* Put `phidget21.dll` in the project folder
* Open `looper.vcxproj` in Visual Studio
* Build and run.
