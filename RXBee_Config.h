

#ifndef RXBEE_CONFIG_H    /* Guard against multiple inclusion */
#define RXBEE_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RXBEE_RX_BUFFER_SIZE
    #define RXBEE_RX_BUFFER_SIZE   (2048)   // Size of receive buffer
#endif


#ifndef RXBEE_MAX_TRANSACTIONS
    #define RXBEE_MAX_TRANSACTIONS 50
#endif

#ifndef RXBEE_DEBUG
    #define RXBEE_DEBUG 0
#endif

#ifndef RXBEE_TRANSACTION_TIMEOUT
    #define RXBEE_TRANSACTION_TIMEOUT 150 
#endif

#ifndef RXBEE_TRANSACTION_RETRY
    #define RXBEE_TRANSACTION_RETRY 2 
#endif

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* RXBEE_CONFIG_H */
