// Based on the simple phidget encoder example

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if (_MSC_VER >= 1600)
#include <windows.h>
#include "ext/phidget21-windevel/phidget21.h"
#pragma comment(lib, "ext/phidget21-windevel/x64/phidget21.lib")
#else
#include <phidget21.h>
#endif


void osc_send(char *address, int i1, int i2) {
    printf("OSC: Send [%d, %d] to \"%s\"\n", i1, i2, address);
}

void osc_init(int port) {
    printf("OSC: Using port #%d\n", port);
}

void osc_kill() {
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
    CPhidget_getSerialNumber(ENC, &serialNo);
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
    CPhidgetHandle handles[100] = { 0, };

    if (argc < 3) {
        printf("Syntax: phidget-encoder-osc-proxy [OSC PORT] [Phidget encoder serial number 1] {Phidget encoder serial number 2} ...");
        return 1;
    }

    int oscport = atoi(argv[1]);
    osc_init(oscport);

    int numdevices = argc - 2;
    for(int i=0; i<numdevices; i++) {
        int serialnum = atoi(argv[2 + i]);
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

