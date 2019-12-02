#include "../include/Global.h"

Packet makePkt(int seqNum, int ackNum, const char *data) {
    Packet pkt;
    pkt.acknum = ackNum;
    pkt.seqnum = seqNum;
    memcpy(pkt.payload, data, Configuration::PAYLOAD_SIZE);
    pkt.checksum = pUtils->calculateCheckSum(pkt);
    return pkt;
}

Packet makeDataPkt(int seqNum, const char *data) {
    return makePkt(seqNum, -1, data);
}

Packet makeAckPkt(int ackNum) {
    char data[Configuration::PAYLOAD_SIZE] = "ACK";
    return makePkt(-1, ackNum, data);
}