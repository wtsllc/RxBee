
#ifndef RXBEE_LOCAL_DEVICE_H
#define RXBEE_LOCAL_DEVICE_H

#include "SpecificDevice.h"
#include "Types.h"

namespace RXBee
{

class LocalDevice : public SpecificDevice<LocalDevice>
{
public:
    LocalDevice();
    ~LocalDevice();
    
    virtual Device::Location GetLocation() const;
    
    LocalDevice* SetPreambleID(uint8_t id);
    LocalDevice* ReadPreambleID();
    LocalDevice* SetNetworkID(uint16_t id);
    LocalDevice* ReadNetworkID();
    
    LocalDevice* NetworkDiscover();
    
    
    LocalDevice* SetApiMode(ApiMode mode);
    LocalDevice* ReadApiMode();
    ApiMode GetApiMode() const;
    
private:
    ApiMode api_mode;
};
    
    
}


#endif // RXBEE_LOCAL_DEVICE_H