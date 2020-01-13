#ifndef TCP_RDT_RECEIVER_H
#define TCP_RDT_RECEIVER_H

#include "RdtReceiver.h"
#include <unordered_map>

class TCPRdtReceiver : public RdtReceiver {
private:
    const unsigned int MAX_SEQ;
    int expectedSeqNum;
    Packet lastAckPkt;
public:
    TCPRdtReceiver(int seqNumBits = 16);
    virtual ~TCPRdtReceiver();
public:
    void receive(const Packet &packet);   // 接收报文，将被NetworkService调用
};

#endif