// Copyright (c) 2018-2021 The Dash Core developers
// Copyright (c) 2020-2023 The Dogetoreum developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <evo/specialtx.h>

#include <chainparams.h>
#include <consensus/validation.h>
#include <hash.h>
#include <primitives/block.h>
#include <validation.h>
#include <evo/cbtx.h>
#include <evo/deterministicmns.h>
#include <llmq/quorums_commitment.h>
#include <llmq/quorums_blockprocessor.h>

bool CheckSpecialTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state,
                    const CCoinsViewCache &view, CAssetsCache *assetsCache, bool check_sigs) {
    if (tx.nVersion != 3 || tx.nType == TRANSACTION_NORMAL)
        return true;

    if (!Params().GetConsensus().DIP0003Enabled) {
        return state.DoS(10, false, REJECT_INVALID, "bad-tx-type");
    }

    try {
        switch (tx.nType) {
            case TRANSACTION_PROVIDER_REGISTER:
                return CheckProRegTx(tx, pindexPrev, state, view, check_sigs);
            case TRANSACTION_PROVIDER_UPDATE_SERVICE:
                return CheckProUpServTx(tx, pindexPrev, state, check_sigs);
            case TRANSACTION_PROVIDER_UPDATE_REGISTRAR:
                return CheckProUpRegTx(tx, pindexPrev, state, view, check_sigs);
            case TRANSACTION_PROVIDER_UPDATE_REVOKE:
                return CheckProUpRevTx(tx, pindexPrev, state, check_sigs);
            case TRANSACTION_COINBASE:
                return CheckCbTx(tx, pindexPrev, state);
            case TRANSACTION_QUORUM_COMMITMENT:
                return llmq::CheckLLMQCommitment(tx, pindexPrev, state);
            case TRANSACTION_FUTURE:
                return CheckFutureTx(tx, pindexPrev, state);
            case TRANSACTION_NEW_ASSET:
                return CheckNewAssetTx(tx, pindexPrev, state, assetsCache);
            case TRANSACTION_UPDATE_ASSET:
                return CheckUpdateAssetTx(tx, pindexPrev, state, view, assetsCache);
            case TRANSACTION_MINT_ASSET:
                return CheckMintAssetTx(tx, pindexPrev, state, view, assetsCache);
        }
    } catch (const std::exception &e) {
        LogPrintf("%s -- failed: %s\n", __func__, e.what());
        return state.DoS(100, false, REJECT_INVALID, "failed-check-special-tx");
    }

    return state.DoS(10, false, REJECT_INVALID, "bad-tx-type-check");
}

bool ProcessSpecialTx(const CTransaction &tx, const CBlockIndex *pindex, CValidationState &state) {
    if (tx.nVersion != 3 || tx.nType == TRANSACTION_NORMAL) {
        return true;
    }

    switch (tx.nType) {
        case TRANSACTION_PROVIDER_REGISTER:
        case TRANSACTION_PROVIDER_UPDATE_SERVICE:
        case TRANSACTION_PROVIDER_UPDATE_REGISTRAR:
        case TRANSACTION_PROVIDER_UPDATE_REVOKE:
            return true; // handled in batches per block
        case TRANSACTION_COINBASE:
            return true; // nothing to do
        case TRANSACTION_QUORUM_COMMITMENT:
            return true; // handled per block
        case TRANSACTION_FUTURE:
            return true;
        case TRANSACTION_NEW_ASSET:
            return true;
        case TRANSACTION_UPDATE_ASSET:
            return true;
        case TRANSACTION_MINT_ASSET:
            return true;
    }
    return state.DoS(100, false, REJECT_INVALID, "bad-tx-type-proc");
}

bool UndoSpecialTx(const CTransaction &tx, const CBlockIndex *pindex) {
    if (tx.nVersion != 3 || tx.nType == TRANSACTION_NORMAL) {
        return true;
    }

    switch (tx.nType) {
        case TRANSACTION_PROVIDER_REGISTER:
        case TRANSACTION_PROVIDER_UPDATE_SERVICE:
        case TRANSACTION_PROVIDER_UPDATE_REGISTRAR:
        case TRANSACTION_PROVIDER_UPDATE_REVOKE:
            return true; // handled in batches per block
        case TRANSACTION_COINBASE:
            return true; // nothing to do
        case TRANSACTION_QUORUM_COMMITMENT:
            return true; // handled per block
        case TRANSACTION_FUTURE:
            return true;
        case TRANSACTION_NEW_ASSET:
            return true;
        case TRANSACTION_UPDATE_ASSET:
            return true;
        case TRANSACTION_MINT_ASSET:
            return true;
    }
    return false;
}

bool ProcessSpecialTxsInBlock(const CBlock &block, const CBlockIndex *pindex, CValidationState &state,
                              const CCoinsViewCache &view, CAssetsCache *assetsCache, bool fJustCheck,
                              bool fCheckCbTxMerleRoots) {
    AssertLockHeld(cs_main);

    try {
        static int64_t nTimeLoop = 0;
        static int64_t nTimeQuorum = 0;
        static int64_t nTimeDMN = 0;
        static int64_t nTimeMerkle = 0;

        int64_t nTime1 = GetTimeMicros();

        for (const auto &ptr_tx: block.vtx) {
            if (!CheckSpecialTx(*ptr_tx, pindex->pprev, state, view, assetsCache, fCheckCbTxMerleRoots)) {
                // pass the state returned by the function above
                return false;
            }
            if (!ProcessSpecialTx(*ptr_tx, pindex, state)) {
                // pass the state returned by the function above
                return false;
            }
        }

        int64_t nTime2 = GetTimeMicros();
        nTimeLoop += nTime2 - nTime1;
        LogPrint(BCLog::BENCHMARK, "        - Loop: %.2fms [%.2fs]\n", 0.001 * (nTime2 - nTime1), nTimeLoop * 0.000001);

        if (!llmq::quorumBlockProcessor->ProcessBlock(block, pindex, state, fJustCheck, fCheckCbTxMerleRoots)) {
            // pass the state returned by the function above
            return false;
        }

        int64_t nTime3 = GetTimeMicros();
        nTimeQuorum += nTime3 - nTime2;
        LogPrint(BCLog::BENCHMARK, "        - quorumBlockProcessor: %.2fms [%.2fs]\n", 0.001 * (nTime3 - nTime2),
                 nTimeQuorum * 0.000001);

        if (!deterministicMNManager->ProcessBlock(block, pindex, state, view, fJustCheck)) {
            // pass the state returned by the function above
            return false;
        }

        int64_t nTime4 = GetTimeMicros();
        nTimeDMN += nTime4 - nTime3;
        LogPrint(BCLog::BENCHMARK, "        - deterministicMNManager: %.2fms [%.2fs]\n", 0.001 * (nTime4 - nTime3),
                 nTimeDMN * 0.000001);

        if (fCheckCbTxMerleRoots && !CheckCbTxMerkleRoots(block, pindex, state, view)) {
            // pass the state returned by the function above
            return false;
        }

        int64_t nTime5 = GetTimeMicros();
        nTimeMerkle += nTime5 - nTime4;
        LogPrint(BCLog::BENCHMARK, "        - CheckCbTxMerkleRoots: %.2fms [%.2fs]\n", 0.001 * (nTime5 - nTime4),
                 nTimeMerkle * 0.000001);
    } catch (const std::exception &e) {
        LogPrintf("%s -- failed: %s\n", __func__, e.what());
        return state.DoS(100, false, REJECT_INVALID, "failed-procspectxsinblock");
    }

    return true;
}

bool UndoSpecialTxsInBlock(const CBlock &block, const CBlockIndex *pindex) {
    AssertLockHeld(cs_main);

    try {
        for (int i = (int) block.vtx.size() - 1; i >= 0; --i) {
            const CTransaction &tx = *block.vtx[i];
            if (!UndoSpecialTx(tx, pindex)) {
                return false;
            }
        }

        if (!deterministicMNManager->UndoBlock(block, pindex)) {
            return false;
        }

        if (!llmq::quorumBlockProcessor->UndoBlock(block, pindex)) {
            return false;
        }
    } catch (const std::exception &e) {
        return error(strprintf("%s -- failed: %s\n", __func__, e.what()).c_str());
    }

    return true;
}

uint256 CalcTxInputsHash(const CTransaction &tx) {
    CHashWriter hw(CLIENT_VERSION, SER_GETHASH);
    for (const auto &in: tx.vin) {
        hw << in.prevout;
    }
    return hw.GetHash();
}
