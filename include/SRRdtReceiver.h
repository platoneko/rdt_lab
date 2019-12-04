#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "RdtReceiver.h"
#include <unordered_map>

class SRRdtReceiver : public RdtReceiver {
private:
    const unsigned int MAX_SEQ;
    const int N;
    int base;
    std::unordered_map<int, Packet> cache;
    inline bool inWindow(int seqNum);
    inline bool inPrevWindow(int seqNum);
public:
    SRRdtReceiver(int n=4, int seqNumBits = 16);
    virtual ~SRRdtReceiver();
public:
    void receive(const Packet &packet);   // 接收报文，将被NetworkService调用
};

#endif
