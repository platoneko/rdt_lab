
#include "../include/Global.h"
#include "../include/GBNRdtSender.h"
#include "../include/utils.h"


GBNRdtSender::GBNRdtSender(int n, int seqNumBits):
    MAX_SEQ((seqNumBits > 0 && seqNumBits <= 16) ? (1 << seqNumBits) : (1 << 16)), 
    N(n)
{
    base = 0;
    nextSeqNum = 0;
}

GBNRdtSender::~GBNRdtSender()
{
}

bool GBNRdtSender::getWaitingState() {
    return nextSeqNum == (base + N) % MAX_SEQ;
}

bool GBNRdtSender::send(const Message &message) {
	if (getWaitingState()) {  // 发送方处于等待确认状态
		return false;
	}
    Packet pkt = makeDataPkt(nextSeqNum, message.data);
    pkts[nextSeqNum] = pkt;
	pUtils->printPacket("发送方发送报文", pkt);
    pns->sendToNetworkLayer(RECEIVER, pkt);
    if (base == nextSeqNum)
    // 启动发送方定时器
	    pns->startTimer(SENDER, Configuration::TIME_OUT, 0);  
    nextSeqNum = (nextSeqNum + 1) % MAX_SEQ;
	return true;
}

void GBNRdtSender::receive(const Packet &ackPkt) {
    printf("发送方窗口：");
    for (int i = base; i != nextSeqNum; i = (i+1)%MAX_SEQ)
        printf("%d ", pkts[i].seqnum);
    printf("\n");
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// 如果校验和正确
	if (checkSum == ackPkt.checksum) {
        base = (ackPkt.acknum + 1) % MAX_SEQ;
        if (pkts.count(ackPkt.acknum)) {
            pkts.erase(ackPkt.acknum);
        }
        pUtils->printPacket("发送方正确收到确认", ackPkt);
        pns->stopTimer(SENDER, 0);
        if (base != nextSeqNum) {
            pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
        }
	}
	else {
		pUtils->printPacket("发送方没有正确收到确认", ackPkt);
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
    int n = nextSeqNum - base;
    if (n < 0) n += MAX_SEQ;
    printf("发送方定时器时间到，重发%d个报文", n);
	// 唯一一个定时器,无需考虑seqNum
    pns->stopTimer(SENDER, 0);
    for (int i = base; i != nextSeqNum; i = (i + 1) % MAX_SEQ) {
	    pUtils->printPacket("重发报文", pkts[i]);
        pns->sendToNetworkLayer(RECEIVER, pkts[i]);
    }
    // 重新启动发送方定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, 0); 
}
