#ifndef SR_RDT_SENDER_H
#define SR_RDT_SENDER_H
#include "RdtSender.h"
#include <unordered_map>

class SRRdtSender : public RdtSender {
private:
    const unsigned int MAX_SEQ;  // 最大序列号+1
    const int N;  // 窗口大小
    int base;
    int nextSeqNum;
    typedef std::pair<Packet, bool> PacketDocker;
    std::unordered_map<int, PacketDocker> pkts;
    inline bool inWindow(int ackNum);  // 判断是否在发送窗口里
public:
    bool send(const Message &message);                  // 发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
    void receive(const Packet &ackPkt);                 // 接受确认Ack，将被NetworkServiceSimulator调用
    void timeoutHandler(int seqNum);                    // Timeout handler，将被NetworkServiceSimulator调用
    bool getWaitingState();
public:
    SRRdtSender(int n = 4, int seqNumBits = 16);
    virtual ~SRRdtSender();
};

#endif