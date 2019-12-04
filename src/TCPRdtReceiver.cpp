#include "../include/TCPRdtReceiver.h"
#include "../include/utils.h"


TCPRdtReceiver::TCPRdtReceiver(int n, int seqNumBits):
    MAX_SEQ((seqNumBits > 0 && seqNumBits <= 16) ? (1 << seqNumBits) : (1 << 16)),
    N(n),
    base(0)
{
}

TCPRdtReceiver::~TCPRdtReceiver()
{
}

inline bool TCPRdtReceiver::inWindow(int seqNum) {
    int start = base;
    int end = (base + N - 1) % MAX_SEQ;
    if (start < end) {
        return start <= seqNum && seqNum <= end;
    } else {
        return seqNum >= start || seqNum <= end;
    }
}

void TCPRdtReceiver::receive(const Packet &packet) {
    int checkSum = pUtils->calculateCheckSum(packet);
    if (checkSum == packet.checksum) {
        pUtils->printPacket("接收方正确收到发送方的报文", packet);
        if (inWindow(packet.seqnum)) {
            printf("在接收方窗口[%d, %d]内\n", base, (base + N - 1)%MAX_SEQ);
            fflush(stdout);
            if (packet.seqnum == base) {
                Message msg;
                memcpy(msg.data, packet.payload, Configuration::PAYLOAD_SIZE);
                pns->delivertoAppLayer(RECEIVER, msg);
                base = (base + 1) % MAX_SEQ;
                while (cache.count(base)) {
                    memcpy(msg.data, cache[base].payload, Configuration::PAYLOAD_SIZE);
                    pns->delivertoAppLayer(RECEIVER, msg);
                    cache.erase(base);
                    base = (base + 1) % MAX_SEQ;
                }
                int ackNum = base - 1;
                if (ackNum < 0) ackNum += MAX_SEQ;
                lastAckPkt = makeAckPkt(ackNum);
                pUtils->printPacket("接收方发送确认报文", lastAckPkt);
                pns->sendToNetworkLayer(SENDER, lastAckPkt);
            } else {
                cache[packet.seqnum] = packet;
            }
        } else {
            pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
	        pns->sendToNetworkLayer(SENDER, lastAckPkt);
        }
    } else {
        pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
    }
    printf("接收方窗口[%d, %d]\n", base, (base + N - 1)%MAX_SEQ);
    fflush(stdout);
}