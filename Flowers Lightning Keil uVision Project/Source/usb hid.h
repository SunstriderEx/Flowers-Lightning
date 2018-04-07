#ifndef USB_H_
#define USB_H_

#include "cmsis_os.h"
#include "rl_usb.h"
#include "stm32f10x_gpio.h"

class USB_HID
{
public:
    enum ReportId
    {
        EnableDeviceUSB,
        DeviceRtcTime,
        DeviceTimeToSleep,
        DeviceTimeToAwake,
        SetRtcTime,
        SetTimeToSleep,
        SetTimeToAwake
    };
    
    static bool enabled;
    //static unsigned int lastCallbackTime;
    //static const uint16_t timeout = 5000; // ms
    
	static void Init();
    static void SendSettings();
    
    //static void CheckUSBConnection();
    
private:
    static void SendReport(uint8_t reportId, uint8_t *data);
};

#endif
