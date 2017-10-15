
#include <stdlib.h>

#include "Network.h"
#include "Transaction.h"
#include "SerialDataSubject.h"
#include "NetworkObserver.h"

namespace RXBee
{

Network::Network()
    : network_id(XBEE_NETWORK_ID_DEFAULT), network_status(ModemStatus::UNKNOWN),
      rx_buff_head_index(0), rx_buff_tail_index(0),
      tx_buff_index(0), frame_count(0),
      frame_count_rollover(0)
{
    local_device.OnTransactionComplete(DeviceTransactionComplete);
}


Network::~Network()
{

}


void Network::Service()
{
    // Send pending transactions
    for (uint16_t i = 0; i < pending.size(); ++i)
    {
        if (pending[i].GetState() == Transaction::State::PENDING)
        {
            const Frame f = pending[i].GetFrame();
            // Write frame to transmit buffer 
            subject.Next(f.Serialize());
            
            // Transaction sent
            pending[i].Sent(frame_count);
            
            // Increment frame count
            if (frame_count == RXBEE_MAX_FRAME_COUNT)
            {
                // Handler overflow
                frame_count = 0;
                frame_count_rollover++;
            }
            else
            {
                frame_count++;
            }
        }
    }
    
    uint16_t buff_max = rx_buff_tail_index;
    
    // Set max buff index to RXBEE_RX_BUFFER_SIZE if the
    // head index is greater than the tail
    if (rx_buff_head_index > rx_buff_tail_index)
    {
        buff_max = RXBEE_RX_BUFFER_SIZE;
    }
    
    // Read from receive buffer until head index equals tail index
    while (rx_buff_head_index != rx_buff_tail_index)
    {
        // Read up to maximum from receive buffer
        if (rx_frame.Deserialize(rx_buff, rx_buff_head_index, buff_max))
        {
            // Complete frame received
            
            if (rx_frame.GetApiID() == ApiID::RECEIVE_PACKET)
            {
                // Received serial data
                std::vector<uint8_t> frame_data;
                uint64_t addr;
                if (rx_frame.GetField(4, addr) &&
                    rx_frame.GetData(15, frame_data))
                {
                    subject.Next(addr, frame_data); // Notify observers
                }
            }
            else
            {
                uint16_t p = 0; 
                for (;p < pending.size(); ++p)
                {
                    // Complete the corresponding transaction
                    if ((pending[p].GetState() == Transaction::State::SENT) &&
                        pending[p].TryComplete(rx_frame))
                    {   
                        break;
                    }
                }

                if (p >= pending.size())
                {
                    // Unsolicited
                    if (rx_frame.GetApiID() == ApiID::AT_COMMAND_RESPONSE)
                    {
                        char cmd[2];
                        uint16_t str_len;
                        rx_frame.GetField(15, cmd, str_len, 2);
                        
                        if (strcmp(cmd, XBEE_CMD_ND) == 0)
                        {
                            uint64_t addr;
                            
                            rx_frame.GetField(5, addr);
                            RemoteDevice dev;
                            dev.SetNetwork(this);
                            dev.SetAddress(addr);
                            remote_devices.push_back(dev);
                        }
                    }
                }
            }
            
            // reset the receive frame
            rx_frame.Clear();
            break;
        }
        
        // Reset head index when it reaches the end of the buffer
        if (rx_buff_head_index >= RXBEE_RX_BUFFER_SIZE)
        {
            rx_buff_head_index = 0;         // Start at front of buffer
            buff_max = rx_buff_tail_index;  // Max is now the tail index
        }
    }
}

uint64_t Network::GetTotalTransactions() const
{
    return frame_count_rollover * RXBEE_MAX_FRAME_COUNT + frame_count;
}


void Network::DiscoverAsync(Network::Callback callback)
{
    disc_comp_cb = callback;
    
}


void Network::OnStatusChanged(Network::Callback callback)
{
    status_changed_cb = callback;
}

Network::ModemStatus Network::GetStatus()
{
    return network_status;
}

bool Network::FindRemoteDevice(const std::string& identifier, RemoteDevice* device)
{
    bool found = false;
    for (uint16_t i = 0; i < remote_devices.size(); ++i)
    {
        if (remote_devices[i].GetIdentifier() == identifier)
        {
            device = &remote_devices[i];
            found = true;
            break;
        }
    }

    return found;
}

bool Network::FindRemoteDevice(const uint64_t address, RemoteDevice* device)
{
    bool found = false;
    for (uint16_t i = 0; i < remote_devices.size(); ++i)
    {
        if (remote_devices[i].GetAddress() == address)
        {
            device = &remote_devices[i];
            found = true;
            break;
        }
    }
    
    return found;
    
}

LocalDevice* Network::GetLocalDevice()
{
    return &local_device;
}

ApiMode Network::GetApiMode()
{
    return local_device.GetApiMode();
}

Transaction* Network::BeginTransaction(Device* device)
{
    uint16_t i = 0; 
    Transaction* t = NULL;
    for (;i < pending.size(); ++i)
    {
        if (pending[i].GetState() == Transaction::State::FREE)
        {
            t = &pending[i];
            break;
        }
    }
    
    if (t == NULL)
    {
        Transaction new_trans;
        
        pending.push_back(new_trans);
        
        t = &pending[pending.size() - 1];
    }
    
    t->Initialize(device);
    
    return t;
}


    
SerialDataSubject* Network::GetSerialDataSubject()
{
    return &subject;
}
    
   
void Network::OnNext(const uint64_t source_addr, const std::vector<uint8_t>& data)
{
    
}

void Network::OnNext(const std::vector<uint8_t>& data)
{
    for(uint16_t i = 0; i < data.size(); ++i)
    {
        rx_buff[rx_buff_tail_index] = data[i];
        
        rx_buff_tail_index++;
        
        if (rx_buff_tail_index >= RXBEE_RX_BUFFER_SIZE)
        {
            rx_buff_tail_index = 0;
        }
    }
}

void Network::OnComplete()
{
    
}

void Network::OnError(const int32_t error_code)
{
    
}


void Network::Subscribe(NetworkObserver* observer)
{
    subscribers.push_back(observer);
}

void Network::DiscoveryComplete()
{
    for (uint16_t i = 0; i < subscribers.size(); ++i)
    {
        subscribers[i]->OnDiscoveryComplete(this);
    }
}

void Network::StatusChanged(Network::ModemStatus status)
{
    network_status = status;
    
    for (uint16_t i = 0; i < subscribers.size(); ++i)
    {
        subscribers[i]->OnStatusChanged(this, status);
    }
}

void Network::DeviceTransactionComplete(Device* device, Transaction* t)
{
    
}

} // namespace XBee