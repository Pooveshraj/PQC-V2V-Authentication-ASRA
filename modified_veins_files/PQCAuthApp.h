// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 The SCA-PQV2V Authors
// Veins application module for SCA-PQV2V, built on Veins 5.2 (GPL-2.0-or-later).

#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include <string>
#include <vector>
#include <chrono>
#include <map>
extern "C" {
#include "vehicle.h"
#include "authentication.h"
#include "session_key.h"
#include "asra.h"
}

namespace veins {

class VEINS_API PQCAuthApp : public DemoBaseApplLayer {
public:
    void initialize(int stage) override;
    void finish() override;
    void handleSelfMsg(cMessage* msg) override;
    ~PQCAuthApp() override;

protected:
    bool sentMessage;
    Vehicle vehicle;
    cOutVector lrsVector;
    cOutVector decisionVector;
    cOutVector signTimeVector;
    cOutVector verifyTimeVector;
    cOutVector asraTimeVector;
    long acceptCount;
    long rejectCount;
    long replayDetectedCount;
    // Within-window replay check (rerun Exp. 4): last accepted signed timestamp per sender public key
    bool seqCheckEnabled;
    long duplicateRejectedCount;
    std::map<std::string, double> lastAcceptedTs;

    bool isAttacker;
    bool asraEnabled;
    bool freshnessEnabled;
    double floodInterval;
    cMessage* floodEvt;
    void sendSignedMessage();

    bool isReplayAttacker;
    double replayDelay;
    cMessage* replayEvt;
    std::string capturedPublicKeyBytes;
    std::string capturedSignatureBytes;
    double capturedTimestamp;
    bool hasCapturedMessage;

    bool isInvalidSigAttacker;
    long invalidSigSentCount;
    double invalidSigInterval;
    cMessage* invalidSigEvt;

protected:
    void onWSM(BaseFrame1609_4* wsm) override;
    void handlePositionUpdate(cObject* obj) override;

private:
    std::string bytesToHex(const uint8_t* data, size_t len);
    std::vector<uint8_t> hexToBytes(const std::string& hex);
};

} // namespace veins
