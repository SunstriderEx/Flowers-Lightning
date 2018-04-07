#include "usb hid.h"
#include "rtc.h"
#include "timing.h"
#include "flash.h"

bool USB_HID::enabled = false;
//unsigned int USB_HID::lastCallbackTime;

void USB_HID::Init()
{
    USBD_Initialize(0); // USB Device 0 Initialization
    //USBD_Connect(0);    // USB Device 0 Connect
}

// Called during USBD_Initialize to initialize the USB Device class.
void USBD_HID0_Initialize()
{
    // Add code for initialization
}


// Called during USBD_Uninitialize to de-initialize the USB Device class.
void USBD_HID0_Uninitialize()
{
    // Add code for de-initialization
}


// \brief Prepare HID Report data to send.
// \param[in]   rtype   report type:
//                - HID_REPORT_INPUT           = input report requested
//                - HID_REPORT_FEATURE         = feature report requested
// \param[in]   req     request type:
//                - USBD_HID_REQ_EP_CTRL       = control endpoint request
//                - USBD_HID_REQ_PERIOD_UPDATE = idle period expiration request
//                - USBD_HID_REQ_EP_INT        = previously sent report on interrupt endpoint request
// \param[in]   rid     report ID (0 if only one report exists).
// \param[out]  buf     buffer containing report data to send.
// \return              number of report data bytes prepared to send or invalid report requested.
//              - value >= 0: number of report data bytes prepared to send
//              - value = -1: invalid report requested
int32_t USBD_HID0_GetReport (uint8_t rtype, uint8_t req, uint8_t rid, uint8_t *buf)
{
    //USB_HID::lastCallbackTime = Timing::Millis();
    
    switch (rtype)
    {
        case HID_REPORT_INPUT:
            //switch (rid) {
            //case 0:
            switch (req)
            {
                case USBD_HID_REQ_EP_CTRL:        // Explicit USB Host request via Control OUT Endpoint
                case USBD_HID_REQ_PERIOD_UPDATE:  // Periodic USB Host request via Interrupt OUT Endpoint
                {
                    // Update buffer for report data, example:
                    // buf[0] = 0;       // Data Value = 0
                    // buf[1] = 5;       // Data Value = 5
                    // return (2);       // Data Size  = 2 bytes
                    return sizeof(buf);
                    //break;
                }
                case USBD_HID_REQ_EP_INT:         // Called after USBD_HID_GetReportTrigger to signal data obtained
                {
                    break;
                }
            }
            break;
            //}
            //break;
 
        case HID_REPORT_FEATURE:
            break;
    }
    return (0);
}
 
 
// \brief Process received HID Report data.
// \param[in]   rtype   report type:
//                - HID_REPORT_OUTPUT    = output report received
//                - HID_REPORT_FEATURE   = feature report received
// \param[in]   req     request type:
//                - USBD_HID_REQ_EP_CTRL = report received on control endpoint
//                - USBD_HID_REQ_EP_INT  = report received on interrupt endpoint
// \param[in]   rid     report ID (0 if only one report exists).
// \param[in]   buf     buffer that receives report data.
// \param[in]   len     length of received report data.
// \return      true    received report data processed.
// \return      false   received report data not processed or request not supported.
bool USBD_HID0_SetReport (uint8_t rtype, uint8_t req, uint8_t rid, const uint8_t *buf, int32_t len)
{
    // Чтобы начать принимать данные сперва нужно что-нибудь отправить
    
    //USB_HID::lastCallbackTime = Timing::Millis();
    
    switch (rtype) {
        case HID_REPORT_OUTPUT:
            /*for (i = 0; i < 8; i++) {
                if (i == 6) continue;
                if (*buf & (1 << i))
                    GPIO_ResetBits(GPIOC, GPIO_Pin_13); // On LED
            else 
                GPIO_SetBits(GPIOC, GPIO_Pin_13); // Off LED
            }*/
            break;
        case HID_REPORT_FEATURE:
        {
            int reportIdSetReport = buf[0];
            int rtcCounterSetReport = (buf[4] << 24) | (buf[3] << 16) | (buf[2] << 8) | (buf[1]);
            switch (reportIdSetReport)
            {
                //case USB_HID::EnableDeviceUSB:
                //{
                //    USB_HID::enabled = true;
                //    break;
                //}
                case USB_HID::SetRtcTime:
                {
                    RTC_SetCounter(rtcCounterSetReport);
                    break;
                }
                case USB_HID::SetTimeToSleep:
                {
                    RTCModule::CounterToTime(rtcCounterSetReport, &RTCModule::TimeToSleep);
                    Flash::StartOrUpdateSettingsWriteTimer();
                    break;
                }
                case USB_HID::SetTimeToAwake:
                {
                    RTCModule::CounterToTime(rtcCounterSetReport, &RTCModule::TimeToAwake);
                    Flash::StartOrUpdateSettingsWriteTimer();
                    break;
                }
            }
            break;
        }
    }
    return true;
}

uint32_t counter;
void USB_HID::SendSettings()
{
    // Чтобы начать принимать данные сперва нужно что-нибудь отправить
    //if (lastCallbackTime - Timing::Millis() >= timeout)
    //    enabled = false;
    
    //if (!enabled)
    //    return;
    
    counter = RTC_GetCounter();
    SendReport(USB_HID::DeviceRtcTime, (uint8_t*)&counter);
    
    osDelay(50); // нужна задержка, иначе посылка будет пропущена. По callback'у не получается
    
    counter = RTCModule::TimeToCounter(&RTCModule::TimeToSleep);
    SendReport(USB_HID::DeviceTimeToSleep, (uint8_t*)&counter);
    
    osDelay(50);
    
    counter = RTCModule::TimeToCounter(&RTCModule::TimeToAwake);
    SendReport(USB_HID::DeviceTimeToAwake, (uint8_t*)&counter);
    
    //USBD_HID_GetReportTrigger(0, USB_HID::DeviceRtcTime, (uint8_t*)&counter, 8);
    //USBD_HID_GetReportTrigger(0, USB_HID::DeviceTimeToSleep, (uint8_t*)&counter, 8);
    //USBD_HID_GetReportTrigger(0, USB_HID::DeviceTimeToAwake, (uint8_t*)&counter, 8);
}

//uint8_t *outputData = new uint8_t[8];
void USB_HID::SendReport(uint8_t reportId, uint8_t *data)
{
    uint8_t *outputData = new uint8_t[8];
    outputData[0] = reportId;
    for (uint8_t i = 0; i < 7; i++)
    {
        if (sizeof(data) > i)
            outputData[i + 1] = data[i];
        else
            outputData[i + 1] = 0;
    }
    
    USBD_HID_GetReportTrigger(0, 0, outputData, 8);
    
    delete []outputData;
}

/*void USB_HID::CheckUSBConnection()
{
    //ESOF: Expected start of frame
    //This bit is set by the hardware when an SOF packet is expected but not received. The host
    //sends an SOF packet each mS, but if the hub does not receive it properly, the Suspend
    //Timer issues this interrupt. If three consecutive ESOF interrupts are generated (i.e. three
    //SOF packets are lost) without any traffic occurring in between, a SUSP interrupt is
    //generated. This bit is set even when the missing SOF packets occur while the Suspend
    //Timer is not yet locked. This bit is read/write but only ‘0 can be written and writing ‘1 has no
    //effect.
    
    // регистр USB->ISTR->ESOF равен "0" при подключении к хосту и "1" в отключенном состоянии
    
    if (...USB_ISTR_ESOF... == 0) // как узнать значение регистра USB->ISTR->ESOF ?
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13); // On LED
        if (!enabled)
        {
            enabled = true;
            USBD_Connect(0); // USB Device 0 Connect
        }
        
    }
    else
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13); // Off LED
        if (enabled)
        {
            enabled = false;
            USBD_Disconnect(0);
        }
    }
}*/



