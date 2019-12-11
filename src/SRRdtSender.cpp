#include "../include/utils.h"
#include "../include/SRRdtSender.h"


SRRdtSender::SRRdtSender(int n, int seqNumBits):
    MAX_SEQ((seqNumBits > 0 && seqNumBits <= 16) ? (1 << seqNumBits) : (1 << 16)), 
    N(n)
{
    base = 0;
    nextSeqNum = 0;
}

SRRdtSender::~SRRdtSender()
{
}

inline bool SRRdtSender::inWindow(int ackNum) {
    if (base == nextSeqNum)  // base == nextSeqNum == 0
        return false;
    if (base < nextSeqNum)
        return base <= ackNum && ackNum < nextSeqNum;
    return nextSeqNum > ackNum || ackNum >= base;  // 序号循环
}

bool SRRdtSender::getWaitingState() {
    return nextSeqNum == (base + N) % MAX_SEQ;
}

bool SRRdtSender::send(const Message &message) {
	if (getWaitingState()) {  // 发送方处于等待确认状态
		return false;
	}
    Packet pkt = makeDataPkt(nextSeqNum, message.data);
    pkts[nextSeqNum] = PacketDocker(pkt, false);
	pUtils->printPacket("发送方发送报文", pkt);
    pns->sendToNetworkLayer(RECEIVER, pkt);
    // 启动发送方定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, nextSeqNum);  
    nextSeqNum = (nextSeqNum + 1) % MAX_SEQ;
    fflush(stdout);
	return true;
}

void SRRdtSender::receive(const Packet &ackPkt) {
    printf("发送方窗口：");
    for (int i = base; i != nextSeqNum; i = (i+1)%MAX_SEQ)
        printf("%d ", pkts[i].first.seqnum);
    printf("\n");    
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// 如果校验和正确
	if (checkSum == ackPkt.checksum) {
        if (inWindow(ackPkt.acknum)) {
            pUtils->printPacket("发送方正确收到确认", ackPkt);
            if (!pkts[ackPkt.acknum].second) {   
                pkts[ackPkt.acknum].second = true;
                pns->stopTimer(SENDER, ackPkt.acknum);
            }
            if (ackPkt.acknum == base) {
                while (base != nextSeqNum && pkts[base].second) {
                    pkts.erase(base);
                    base = (base + 1) % MAX_SEQ;
                }
            }
        } else {
            pUtils->printPacket("发送方正确收到确认，但未在窗口内", ackPkt);  
        }
	} else {
		pUtils->printPacket("发送方没有正确收到确认", ackPkt);
	}
    fflush(stdout);
}

void SRRdtSender::timeoutHandler(int seqNum) {
    pns->stopTimer(SENDER, seqNum);
    pns->sendToNetworkLayer(RECEIVER, pkts[seqNum].first);
    pUtils->printPacket("发送方定时器时间到，重发超时的报文", pkts[seqNum].first);
    pns->startTimer(SENDER, Configuration::TIME_OUT, seqNum);
}
