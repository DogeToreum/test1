// Copyright (c) 2018-2021 The Dash Core developers
// Copyright (c) 2020-2023 The Dogetoreum developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DOGETOREUM_QUORUMS_INIT_H
#define DOGETOREUM_QUORUMS_INIT_H

class CConnman;

class CDBWrapper;

class CEvoDB;

class CTxMemPool;

namespace llmq {

// Init/destroy LLMQ globals
    void InitLLMQSystem(CEvoDB &evoDb, CTxMemPool &mempool, CConnman &connman, bool unitTests, bool fWipe = false);

    void DestroyLLMQSystem();

// Manage scheduled tasks, threads, listeners etc.
    void StartLLMQSystem();

    void StopLLMQSystem();

    void InterruptLLMQSystem();
} // namespace llmq

#endif //DOGETOREUM_QUORUMS_INIT_H
