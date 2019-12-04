#include "../include/utils.h"
#include "../include/TCPRdtSender.h"

TCPRdtSender::TCPRdtSender(int n, int seqNumBits):
    MAX_SEQ((seqNumBits > 0 && seqNumBits <= 16) ? (1 << seqNumBits) : (1 << 16)), 
    N(n)
{
}

TCPRdtSender::~TCPRdtSender()
{
}

inline bool TCPRdtSender::inWindow(int ackNum) {
    if (base == nextSeqNum)  // base == nextSeqNum == 0
        return false;
    if (base < nextSeqNum)
        return base <= ackNum && ackNum < nextSeqNum;
    return nextSeqNum > ackNum || ackNum >= base;  // 序号循环
}

bool TCPRdtSender::getWaitingState() {
    return nextSeqNum == (base + N) % MAX_SEQ;
}

bool TCPRdtSender::send(const Message &message) {
	if (getWaitingState()) {  // 发送方处于等待确认状态
        printf("发送方窗口[%d, %d]\n", base, nextSeqNum);
		return false;
	}
    Packet pkt = makeDataPkt(nextSeqNum, message.data);
    pkts[nextSeqNum] = pkt;
	pUtils->printPacket("发送方发送报文", pkt);
    pns->sendToNetworkLayer(RECEIVER, pkt);
    // 启动发送方定时器
    if (nextSeqNum == base) {
    	pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
    }
    nextSeqNum = (nextSeqNum + 1) % MAX_SEQ;
    printf("发送方窗口[%d, %d]\n", base, nextSeqNum);
    fflush(stdout);
	return true;
}

void TCPRdtSender::receive(const Packet &ackPkt) {
	// 检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	// 如果校验和正确
	if (checkSum == ackPkt.checksum) {
        if (inWindow(ackPkt.acknum)) {
            pUtils->printPacket("发送方正确收到确认", ackPkt);
            while (base != ackPkt.acknum) {  // 清除缓存
                pkts.erase(base);
                base = (base + 1) % MAX_SEQ;
            }
            pkts.erase(base);                // 清除缓存
            base = (base + 1) % MAX_SEQ;
            ackCount = 0;
            pns->stopTimer(SENDER, 0);
            if (base != nextSeqNum) {
                pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
            }      
        } else {
            pUtils->printPacket("发送方收到之前的确认", ackPkt);
            ++ackCount;
            if (ackCount == 3) {
                pns->sendToNetworkLayer(RECEIVER, pkts[base]);
                pUtils->printPacket("收到3个冗余ACK，快速重传报文", pkts[base]);
                ackCount = 0;               
            } 
        }
	} else {
		pUtils->printPacket("发送方没有正确收到确认", ackPkt);
	}
    printf("发送方窗口[%d, %d]\n", base, nextSeqNum);
    fflush(stdout);
}

void TCPRdtSender::timeoutHandler(int seqNum) {
    pns->stopTimer(SENDER, 0);
    pns->sendToNetworkLayer(RECEIVER, pkts[base]);
    pUtils->printPacket("发送方定时器时间到，重发超时的报文", pkts[base]);
    pns->startTimer(SENDER, Configuration::TIME_OUT, 0);
}