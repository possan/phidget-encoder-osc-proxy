// Based on the simple phidget encoder example

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if (_MSC_VER >= 1600)

#include <windows.h>
#include "ext/phidget21-windevel/phidget21.h"
#pragma comment(lib, "ext/phidget21-windevel/x64/phidget21.lib")

#else

#include <unistd.h>
#include <phidget21.h>

#endif

#include <iostream>
#define OSCPKT_OSTREAM_OUTPUT
#include "ext/libtinyosc/tinyosc.hh"
#include "ext/libtinyosc/oscudp.hh"

using namespace oscpkt;

UdpSocket oscsock;

void osc_send(const char *address, int i1, int i2) {
    Message msg(address);
    msg.pushInt32(i1);
    msg.pushInt32(i2);

    PacketWriter pw;
    pw.startBundle().startBundle().addMessage(msg).endBundle().endBundle();

    bool ok = oscsock.sendPacket(pw.packetData(), pw.packetSize());
    printf("OSC: Sent [%d, %d] to \"%s\" (%d)\n", i1, i2, address, ok);
}

void osc_init(const char *host, int port) {
    printf("OSC: Using \"%s:%d\"\n", host, port);
	oscsock.connectTo(host, port);
    if (!oscsock.isOk()) {
		printf("OSC: Failed to open port: %s", oscsock.errorMessage().c_str());
	};
}

void osc_kill() {
    oscsock.close();
}



int CCONV AttachHandler(CPhidgetHandle ENC, void *userptr) {
    int serialNo;
    CPhidget_DeviceID deviceID;
    int i, inputcount;
    CPhidget_getSerialNumber(ENC, &serialNo);
    CPhidget_getDeviceID(ENC, &deviceID);
    CPhidgetEncoder_getEncoderCount((CPhidgetEncoderHandle)ENC, &inputcount);
    printf("Encoder %10d attached! \n", serialNo);
    if (deviceID == PHIDID_ENCODER_HS_4ENCODER_4INPUT) {
        printf("Encoder requires Enable. Enabling inputs....\n");
        for (i = 0 ; i < inputcount ; i++)
            CPhidgetEncoder_setEnabled((CPhidgetEncoderHandle)ENC, i, 1);
    }
    return 0;
}

int CCONV DetachHandler(CPhidgetHandle ENC, void *userptr) {
    int serialNo;
    CPhidget_getSerialNumber(ENC, &serialNo);
    printf("Encoder %10d detached! \n", serialNo);
    return 0;
}

int CCONV ErrorHandler(CPhidgetHandle ENC, void *userptr, int ErrorCode, const char *Description) {
    printf("Error handled. %d - %s \n", ErrorCode, Description);
    return 0;
}

int CCONV PositionChangeHandler(CPhidgetEncoderHandle ENC, void *usrptr, int Index, int Time, int RelativePosition) {
    int Position;
    int serialNo;
    CPhidget_getSerialNumber((CPhidgetHandle)ENC, &serialNo);
    CPhidgetEncoder_getPosition(ENC, Index, &Position);
    // printf("Serial #%d - Encoder #%i - Position: %5d -- (Relative %2d)\n", serialNo, Index, Position, RelativePosition);
    char addr[100];
    sprintf(addr, "/phidgets/%d/encoder/%d", serialNo, Index);
    osc_send(addr, Position, RelativePosition);
    return 0;
}

void stop_listening(CPhidgetEncoderHandle encoder) {
    printf("Closing...\n");
    CPhidget_close((CPhidgetHandle)encoder);
    CPhidget_delete((CPhidgetHandle)encoder);
}

CPhidgetEncoderHandle start_listening(int serialnumber) {
    int result;
    const char *err;
    CPhidgetEncoderHandle encoder = 0;
    printf("Looking for encoder with serialnumber #%d...\n", serialnumber);
    CPhidgetEncoder_create(&encoder);
    CPhidget_set_OnAttach_Handler((CPhidgetHandle)encoder, AttachHandler, NULL);
    CPhidget_set_OnDetach_Handler((CPhidgetHandle)encoder, DetachHandler, NULL);
    CPhidget_set_OnError_Handler((CPhidgetHandle)encoder, ErrorHandler, NULL);
    CPhidgetEncoder_set_OnPositionChange_Handler (encoder, PositionChangeHandler, NULL);
    CPhidget_open((CPhidgetHandle)encoder, serialnumber);
    return encoder;
}

int main(int argc, char* argv[])
{
    CPhidgetEncoderHandle handles[100] = { 0, };

    if (argc < 4) {
        printf("Syntax: phidget-encoder-osc-proxy [OSC Hostname] [OSC Port] [Phidget encoder serial number 1] {Phidget encoder serial number 2} ...");
        return 1;
    }

    const char *oscip = argv[1];
    int oscport = atoi(argv[2]);
    osc_init(oscip, oscport);

    int numdevices = argc - 3;
    for(int i=0; i<numdevices; i++) {
        int serialnum = atoi(argv[3 + i]);
        handles[i] = start_listening(serialnum);
    }

    while(1) {
        printf("Chilling...\n");
#if (_MSC_VER >= 1600)
        Sleep(2000);
#else
        sleep(2);
#endif
    }

    for(int i=0; i<numdevices; i++) {
        stop_listening(handles[i]);
    }

    osc_kill();

    return 0;
}

