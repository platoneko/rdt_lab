#include "../include/TCPRdtReceiver.h"
#include "../include/utils.h"


TCPRdtReceiver::TCPRdtReceiver(int seqNumBits):
    MAX_SEQ((seqNumBits > 0 && seqNumBits <= 16) ? (1 << seqNumBits) : (1 << 16))
{
    expectedSeqNum = 0;
    lastAckPkt = makeAckPkt(0);
}

TCPRdtReceiver::~TCPRdtReceiver()
{
}

void TCPRdtReceiver::receive(const Packet &packet) {
    int checkSum = pUtils->calculateCheckSum(packet);
    if (checkSum == packet.checksum && expectedSeqNum == packet.seqnum) {
        pUtils->printPacket("接收方正确收到发送方的报文", packet);
        Message msg;
        memcpy(msg.data, packet.payload, sizeof(packet.payload));
        pns->delivertoAppLayer(RECEIVER, msg);
        expectedSeqNum = (expectedSeqNum + 1) % MAX_SEQ;
        lastAckPkt = makeAckPkt(expectedSeqNum);
        pUtils->printPacket("接收方发送确认报文", lastAckPkt);
    } else {
        if (checkSum != packet.checksum)
            pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
        else
            pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);
        pUtils->printPacket("接收方重新发送上次的确认报文", lastAckPkt);
    }
    // 调用模拟网络环境的sendToNetworkLayer，通过网络层发送上次的确认报文
	pns->sendToNetworkLayer(SENDER, lastAckPkt);
}