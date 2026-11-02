// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (c) 2026 The SCA-PQV2V Authors
// Veins application module for SCA-PQV2V, built on Veins 5.2 (GPL-2.0-or-later).

package org.car2x.veins.modules.application.pqcauth;
import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple PQCAuthApp extends DemoBaseApplLayer
{
    @class(veins::PQCAuthApp);
    @display("i=block/app2");
    bool isAttacker = default(false);
    double floodInterval @unit(s) = default(1s);
    bool isReplayAttacker = default(false);
    double replayDelay @unit(s) = default(2s);
    bool isInvalidSigAttacker = default(false);
    double invalidSigInterval @unit(s) = default(1.5s);
    bool asraEnabled = default(true);
    bool freshnessEnabled = default(true);
    bool seqCheckEnabled = default(false);
}
