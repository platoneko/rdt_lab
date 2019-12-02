#ifndef UTILS_H
#define UTILS_H

#include "Global.h"
Packet makePkt(int seqNum, int ackNum, const char *data);
Packet makeDataPkt(int seqNum, const char *data);
Packet makeAckPkt(int ackNum);
#endif