
#ifndef RXBEE_NETOWRK_H
#define RXBEE_NETOWRK_H

#include <stdint.h>
#include <vector>
#include <string>

#include "Frame.h"
#include "Command.h"
#include "SerialDataObserver.h"
#include "SerialDataSubject.h"
#include "Transaction.h"
#include "Types.h"
#include "SpecificResponses.h"



namespace RXBee
{

class NetworkObserver;

class XBeeNetwork : public SerialDataObserver
{
public:
    typedef void (*Callback)(XBeeNetwork* source);
    typedef void (*PrintCallback)(const char* message);

    
    
    enum class DeliveryStatus
    {
        SUCCESS = 0x00,
        MAC_ACK_FAIL = 0x01,
        COLLISION_AVOIDANCE_FAIL = 0x02,
        NETWORK_ACK_FAIL = 0x21,
        ROUTE_NOT_FOUND = 0x25,
        INTERNAL_RESOURCE_ERROR = 0x31,
        INTERNAL_ERROR = 0x32,
        PAYLOAD_TOO_LARGE = 0x74,
        INDIRECT_MSG_REQUESTED = 0x75
    };
    
    enum class CommandStatus
    {
        OK = 0x00,
        ERROR = 0x01,
        INVALID_COMMAND = 0x02,
        INVALID_PARAMETER = 0x03,
        TX_FAILURE = 0x04
    };

    XBeeNetwork();
    virtual ~XBeeNetwork();

    void Service(uint32_t milliseconds);

    Transaction* DiscoverAsync();

    void OnStatusChanged(Callback callback);
    
    ModemStatus GetStatus();
    
    uint16_t GetNetworkID() const;
    uint8_t GetPreambleID() const; 
    Address GetLocalAddress() const;
    
    ApiMode GetApiMode();
    
    Transaction* BeginTransaction(Address addr);
    Transaction* BeginTransaction();
    Transaction* BeginBroadcastTransaction();
    
    uint64_t GetTotalTransactions() const;
    
    SerialDataSubject* GetSerialDataSubject();
       
    void OnNext(const std::vector<uint8_t>& data);
    void OnNext(const uint8_t* data, const uint16_t len);
    void OnComplete();
    void OnError(const int32_t error_code);
    
    void Subscribe(NetworkObserver* observer);
    
    void RegisterPrintHandler(PrintCallback handler);
    
    void Print(const char* msg);
    
    uint16_t GetMaxPacketPayloadBytes() const;

protected:
    
    virtual void DeviceDiscovered(Address address, const std::string& node_id);
    
    virtual void StatusChanged(ModemStatus status);
    
    virtual void SerialDataReceived(const uint64_t source_addr, const std::vector<uint8_t>& data);
    
private:
    
    ModemStatus network_status;
    
    Frame rx_frame;
    
    uint8_t frame_count;
    uint32_t frame_count_rollover;
    
    std::vector<Transaction*> pending;
        
    std::vector<NetworkObserver*> subscribers;
    
    uint8_t rx_buff[RXBEE_RX_BUFFER_SIZE];
    
    uint16_t rx_buff_head_index;
    uint16_t rx_buff_tail_index;
    uint16_t tx_buff_index;
    
    SerialDataSubject subject;
    
    Callback disc_comp_cb;
    Callback status_changed_cb;
    PrintCallback print_handler;
    
    Address local_addr;
    ApiMode api_mode;
    char node_identifier[XBEE_AT_NI_IDENT_LEN];
    uint8_t preamble_id;
    uint16_t network_id;
    
    uint16_t max_packet_payload_bytes;
};

} // namespace XBee

#endif // RXBEE_NETWORK_H