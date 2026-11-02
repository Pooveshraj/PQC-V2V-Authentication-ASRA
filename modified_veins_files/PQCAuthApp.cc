// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 The SCA-PQV2V Authors
// Veins application module for SCA-PQV2V, built on Veins 5.2 (GPL-2.0-or-later).

#include "veins/modules/application/pqcauth/PQCAuthApp.h"
#include "veins/modules/application/pqcauth/PQCAuthMessage_m.h"

#include <sstream>
#include <iomanip>

using namespace veins;

Define_Module(veins::PQCAuthApp);

std::string PQCAuthApp::bytesToHex(const uint8_t* data, size_t len)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; i++) {
        oss << std::setw(2) << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::vector<uint8_t> PQCAuthApp::hexToBytes(const std::string& hex)
{
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i + 1 < hex.length(); i += 2) {
        uint8_t byte = (uint8_t)strtol(hex.substr(i, 2).c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

void PQCAuthApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    if (stage == 0) {
        sentMessage = false;
        acceptCount = 0;
        rejectCount = 0;
        replayDetectedCount = 0;
        duplicateRejectedCount = 0;
        lrsVector.setName("LRS");
        decisionVector.setName("decision");
        signTimeVector.setName("signTimeUs");
        verifyTimeVector.setName("verifyTimeUs");
        asraTimeVector.setName("asraTimeUs");

        isAttacker = par("isAttacker").boolValue();
        asraEnabled = par("asraEnabled").boolValue();
        freshnessEnabled = par("freshnessEnabled").boolValue();
        seqCheckEnabled = par("seqCheckEnabled").boolValue();
        floodInterval = par("floodInterval").doubleValue();
        floodEvt = nullptr;

        isReplayAttacker = par("isReplayAttacker").boolValue();
        replayDelay = par("replayDelay").doubleValue();
        replayEvt = nullptr;
        hasCapturedMessage = false;

        isInvalidSigAttacker = par("isInvalidSigAttacker").boolValue();
        invalidSigSentCount = 0;
        invalidSigInterval = par("invalidSigInterval").doubleValue();
        invalidSigEvt = nullptr;

        std::string vehicleId = getParentModule()->getFullName();
        initialize_vehicle(&vehicle, vehicleId.c_str());

        if (!generate_dsa_keys(&vehicle)) {
            EV << "PQCAuthApp: DSA key generation FAILED for [" << vehicleId << "]\n";
        }
        if (!generate_kem_keys(&vehicle)) {
            EV << "PQCAuthApp: KEM key generation FAILED for [" << vehicleId << "]\n";
        }

        EV << "PQCAuthApp initialized with real ML-DSA/ML-KEM keys for vehicle: " << vehicleId << "\n";

        if (isAttacker) {
            floodEvt = new cMessage("floodEvt");
            scheduleAt(simTime() + uniform(0, floodInterval), floodEvt);
            EV << "PQCAuthApp: [" << vehicleId << "] configured as FLOODING ATTACKER, interval=" << floodInterval << "s\n";
        }

        if (isInvalidSigAttacker) {
            invalidSigEvt = new cMessage("invalidSigEvt");
            scheduleAt(simTime() + uniform(0, invalidSigInterval), invalidSigEvt);
            EV << "PQCAuthApp: [" << vehicleId << "] configured as INVALID-SIG ATTACKER, interval=" << invalidSigInterval << "s\n";
        }
    }
}

void PQCAuthApp::handleSelfMsg(cMessage* msg)
{
    if (msg == floodEvt) {
        sendSignedMessage();
        scheduleAt(simTime() + floodInterval, floodEvt);
        return;
    }
    if (msg == invalidSigEvt) {
        sendSignedMessage();
        scheduleAt(simTime() + invalidSigInterval, invalidSigEvt);
        return;
    }
    if (msg == replayEvt) {
        if (hasCapturedMessage) {
            PQCAuthMessage* wsm = new PQCAuthMessage();
            populateWSM(wsm);
            wsm->setSenderId(vehicle.vehicle_id);
            wsm->setPublicKeyBytes(capturedPublicKeyBytes.c_str());
            wsm->setSignatureBytes(capturedSignatureBytes.c_str());
            wsm->setKemCiphertextBytes("");
            wsm->setTimestamp(capturedTimestamp);

            EV << "PQCAuthApp: [" << vehicle.vehicle_id << "] REPLAYING captured message, "
               << "original_timestamp=" << capturedTimestamp
               << " current_time=" << simTime().dbl() << "\n";
            sendDown(wsm);

            scheduleAt(simTime() + replayDelay, replayEvt);
        }
        return;
    }
    DemoBaseApplLayer::handleSelfMsg(msg);
}

void PQCAuthApp::finish()
{
    recordScalar("acceptCount", acceptCount);
    recordScalar("rejectCount", rejectCount);
    recordScalar("replayDetectedCount", replayDetectedCount);
    recordScalar("duplicateRejectedCount", duplicateRejectedCount);
    recordScalar("invalidSigSentCount", invalidSigSentCount);
    DemoBaseApplLayer::finish();
}

PQCAuthApp::~PQCAuthApp()
{
    if (floodEvt) {
        cancelAndDelete(floodEvt);
    }
    if (replayEvt) {
        cancelAndDelete(replayEvt);
    }
    if (invalidSigEvt) {
        cancelAndDelete(invalidSigEvt);
    }
    free_vehicle(&vehicle);
}

void PQCAuthApp::onWSM(BaseFrame1609_4* frame)
{
    PQCAuthMessage* wsm = check_and_cast<PQCAuthMessage*>(frame);

    std::string senderId = wsm->getSenderId();
    std::vector<uint8_t> pubKeyBytes = hexToBytes(wsm->getPublicKeyBytes());
    std::vector<uint8_t> sigBytes = hexToBytes(wsm->getSignatureBytes());
    double msgTimestamp = wsm->getTimestamp();

    std::ostringstream challengeStream;
    challengeStream << "V2V_AUTH_CHALLENGE:" << std::fixed << std::setprecision(6) << msgTimestamp;
    std::string challenge = challengeStream.str();

    const double FRESHNESS_WINDOW_SEC = 1.0;
    double age = simTime().dbl() - msgTimestamp;
    bool fresh = (age >= 0.0 && age <= FRESHNESS_WINDOW_SEC);

    auto verifyStart = std::chrono::high_resolution_clock::now();
    bool valid = verify_signature(pubKeyBytes.data(),
                                   (const uint8_t*)challenge.c_str(),
                                   challenge.length(),
                                   sigBytes.data(),
                                   sigBytes.size());
    auto verifyEnd = std::chrono::high_resolution_clock::now();
    double verifyUs = std::chrono::duration<double, std::micro>(verifyEnd - verifyStart).count();
    verifyTimeVector.record(verifyUs);

    if (!fresh) {
        replayDetectedCount++;
    }
    // Ablation: when freshnessEnabled is false, replay/timestamp checks are
    // still logged above (for observability) but do NOT affect the actual
    // accept/reject decision -- valid depends only on signature correctness.
    if (freshnessEnabled) {
        valid = valid && fresh;
    }

    // Within-window replay check. The timestamp is inside the signed challenge,
    // so it acts as a per-sender monotone sequence value. A message whose signed
    // timestamp is not newer than the last accepted one from the same public key
    // is a duplicate and is rejected, even if it is still inside the window W.
    const std::string senderKey = wsm->getPublicKeyBytes();
    if (seqCheckEnabled) {
        auto it = lastAcceptedTs.find(senderKey);
        if (it != lastAcceptedTs.end() && msgTimestamp <= it->second) {
            duplicateRejectedCount++;
            valid = false;
        }
        // Update only after the signature and freshness checks pass, so a
        // forged or stale message cannot advance the stored value.
        if (valid) {
            lastAcceptedTs[senderKey] = msgTimestamp;
        }
    }

    bool accepted;
    double lrsValue = 0.0;  // no meaningful LRS when ASRA is disabled (ablation)

    if (asraEnabled) {
        auto asraStart = std::chrono::high_resolution_clock::now();
        ASRA_Result risk = asra_evaluate_signing_robust(&vehicle, 3, 20);
        auto asraEnd = std::chrono::high_resolution_clock::now();
        double asraUs = std::chrono::duration<double, std::micro>(asraEnd - asraStart).count();
        asraTimeVector.record(asraUs);

        accepted = asra_authentication_decision(valid, &risk, 0.7);
        lrsValue = risk.leakage_risk_score;
    } else {
        // Ablation: baseline PQC authentication only -- accept purely on
        // cryptographic validity + freshness, bypassing ASRA's risk-scoring
        // layer entirely, mirroring how a non-ASRA baseline (e.g. Yadav
        // CAT/CLAT) would decide.
        accepted = valid;
    }

    lrsVector.record(lrsValue);
    decisionVector.record(accepted ? 1 : 0);
    if (accepted) {
        acceptCount++;
    } else {
        rejectCount++;
    }

    EV << "PQCAuthApp: received message from [" << senderId << "] "
       << "sig_valid=" << valid
       << " LRS=" << lrsValue
       << " decision=" << (accepted ? "ACCEPT" : "REJECT") << "\n";
}

void PQCAuthApp::handlePositionUpdate(cObject* obj)
{
    DemoBaseApplLayer::handlePositionUpdate(obj);

    if (!sentMessage) {
        sentMessage = true;
        sendSignedMessage();
    }
}

void PQCAuthApp::sendSignedMessage()
{
    double now = simTime().dbl();
    std::ostringstream challengeStream;
    challengeStream << "V2V_AUTH_CHALLENGE:" << std::fixed << std::setprecision(6) << now;
    std::string challenge = challengeStream.str();

    uint8_t* signature = nullptr;
    size_t sig_len = 0;

    auto signStart = std::chrono::high_resolution_clock::now();
    bool signed_ok = sign_message(&vehicle,
                                   (const uint8_t*)challenge.c_str(),
                                   challenge.length(),
                                   &signature,
                                   &sig_len);
    auto signEnd = std::chrono::high_resolution_clock::now();
    double signUs = std::chrono::duration<double, std::micro>(signEnd - signStart).count();
    signTimeVector.record(signUs);

    if (!signed_ok) {
        EV << "PQCAuthApp: signing FAILED for [" << vehicle.vehicle_id << "]\n";
        return;
    }

    std::string sigHex = bytesToHex(signature, sig_len);
    if (isInvalidSigAttacker) {
        // Corrupt a handful of hex characters to invalidate the signature
        // while keeping it structurally well-formed (same length).
        for (int i = 0; i < 8 && i < (int)sigHex.length(); i++) {
            size_t pos = (i * 37) % sigHex.length();
            char c = sigHex[pos];
            sigHex[pos] = (c == 'f') ? '0' : 'f';
        }
        invalidSigSentCount++;
        EV << "PQCAuthApp: [" << vehicle.vehicle_id << "] sending CORRUPTED signature (invalid-sig attack)\n";
    }

    PQCAuthMessage* wsm = new PQCAuthMessage();
    populateWSM(wsm);
    wsm->setSenderId(vehicle.vehicle_id);
    wsm->setPublicKeyBytes(bytesToHex(vehicle.dsa_public_key, OQS_SIG_ml_dsa_65_length_public_key).c_str());
    wsm->setSignatureBytes(sigHex.c_str());
    wsm->setKemCiphertextBytes("");
    wsm->setTimestamp(now);

    EV << "PQCAuthApp: sending signed auth message from [" << vehicle.vehicle_id << "]\n";

    if (isReplayAttacker && !hasCapturedMessage) {
        capturedPublicKeyBytes = bytesToHex(vehicle.dsa_public_key, OQS_SIG_ml_dsa_65_length_public_key);
        capturedSignatureBytes = bytesToHex(signature, sig_len);
        capturedTimestamp = now;
        hasCapturedMessage = true;

        replayEvt = new cMessage("replayEvt");
        scheduleAt(simTime() + replayDelay, replayEvt);
        EV << "PQCAuthApp: [" << vehicle.vehicle_id << "] captured message for replay, will resend in "
           << replayDelay << "s\n";
    }

    sendDown(wsm);

    free(signature);
}
