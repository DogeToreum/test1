// Copyright (c) 2018-2021 The Dash Core developers
// Copyright (c) 2020-2023 The Dogetoreum developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_EVO_PROVIDERTX_H
#define BITCOIN_EVO_PROVIDERTX_H

#include <bls/bls.h>
#include <primitives/transaction.h>

#include <key_io.h>
#include <netaddress.h>
#include <pubkey.h>
#include <univalue.h>

class CBlockIndex;

class CCoinsViewCache;

class CValidationState;

class CAssetsCache;

class CProRegTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION};                    // message version
    uint16_t nType{0};                                     // only 0 supported for now
    uint16_t nMode{0};                                     // only 0 supported for now
    COutPoint collateralOutpoint{uint256(), (uint32_t) - 1}; // if hash is null, we refer to a ProRegTx output
    CService addr;
    CKeyID keyIDOwner;
    CBLSPublicKey pubKeyOperator;
    CKeyID keyIDVoting;
    uint16_t nOperatorReward{0};
    CScript scriptPayout;
    uint256 inputsHash; // replay protection
    std::vector<unsigned char> vchSig;

    SERIALIZE_METHODS(CProRegTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.nType, obj.nMode, obj.collateralOutpoint,
                  obj.addr, obj.keyIDOwner, obj.pubKeyOperator, obj.keyIDVoting,
                  obj.nOperatorReward, obj.scriptPayout, obj.inputsHash);
        if (!(s.GetType() & SER_GETHASH)) {
            READWRITE(obj.vchSig);
        }
    }

    // When signing with the collateral key, we don't sign the hash but a generated message instead
    // This is needed for HW wallet support which can only sign text messages as of now
    std::string MakeSignString() const;

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("collateralHash", collateralOutpoint.hash.ToString());
        obj.pushKV("collateralIndex", (int) collateralOutpoint.n);
        obj.pushKV("service", addr.ToString(false));
        obj.pushKV("ownerAddress", EncodeDestination(keyIDOwner));
        obj.pushKV("votingAddress", EncodeDestination(keyIDVoting));

        CTxDestination dest;
        if (ExtractDestination(scriptPayout, dest)) {
            obj.pushKV("payoutAddress", EncodeDestination(dest));
        }
        obj.pushKV("pubKeyOperator", pubKeyOperator.ToString());
        obj.pushKV("operatorReward", (double) nOperatorReward / 100);

        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

class CProUpServTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION}; // message version
    uint256 proTxHash;
    CService addr;
    CScript scriptOperatorPayout;
    uint256 inputsHash; // replay protection
    CBLSSignature sig;

    SERIALIZE_METHODS(CProUpServTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.proTxHash, obj.addr, obj.scriptOperatorPayout, obj.inputsHash);
        if (!(s.GetType() & SER_GETHASH)) {
            READWRITE(obj.sig);
        }
    }

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("proTxHash", proTxHash.ToString());
        obj.pushKV("service", addr.ToString(false));
        CTxDestination dest;
        if (ExtractDestination(scriptOperatorPayout, dest)) {
            obj.pushKV("operatorPayoutAddress", EncodeDestination(dest));
        }
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

class CProUpRegTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION}; // message version
    uint256 proTxHash;
    uint16_t nMode{0}; // only 0 supported for now
    CBLSPublicKey pubKeyOperator;
    CKeyID keyIDVoting;
    CScript scriptPayout;
    uint256 inputsHash; // replay protection
    std::vector<unsigned char> vchSig;

    SERIALIZE_METHODS(CProUpRegTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.proTxHash, obj.nMode, obj.pubKeyOperator,
                  obj.keyIDVoting, obj.scriptPayout, obj.inputsHash);
        if (!(s.GetType() & SER_GETHASH)) {
            READWRITE(obj.vchSig);
        }
    }

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("proTxHash", proTxHash.ToString());
        obj.pushKV("votingAddress", EncodeDestination(keyIDVoting));
        CTxDestination dest;
        if (ExtractDestination(scriptPayout, dest)) {
            obj.pushKV("payoutAddress", EncodeDestination(dest));
        }
        obj.pushKV("pubKeyOperator", pubKeyOperator.ToString());
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

class CProUpRevTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    // these are just informational and do not have any effect on the revocation
    enum {
        REASON_NOT_SPECIFIED = 0,
        REASON_TERMINATION_OF_SERVICE = 1,
        REASON_COMPROMISED_KEYS = 2,
        REASON_CHANGE_OF_KEYS = 3,
        REASON_LAST = REASON_CHANGE_OF_KEYS
    };

    uint16_t nVersion{CURRENT_VERSION}; // message version
    uint256 proTxHash;
    uint16_t nReason{REASON_NOT_SPECIFIED};
    uint256 inputsHash; // replay protection
    CBLSSignature sig;

    SERIALIZE_METHODS(CProUpRevTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.proTxHash, obj.nReason, obj.inputsHash);
        if (!(s.GetType() & SER_GETHASH)) {
            READWRITE(obj.sig);
        }
    }

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("proTxHash", proTxHash.ToString());
        obj.pushKV("reason", (int) nReason);
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

/**
 * Future transaction is a transaction locks by maturity (number of block confirmation) or
 * by lockTime (number of second from its first confirm time) whichever come later.
 * if maturity is negative, this transaction lock by lockTime.
 * If lockTime is negative, this transaction lock by maturity.
 * If both are negative, this transaction is locked till external condition is met
 *
 */
class CFutureTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION};// message version
    int32_t maturity; // number of confirmations to be matured and spendable.
    int32_t lockTime; // number of seconds for this transaction to be spendable
    uint16_t lockOutputIndex; // vout index that is locked in this transaction
    uint16_t fee; // fee was paid for this future in addition to miner fee. it is a whole non-decimal point value.
    bool updatableByDestination = false; // true to allow some information of this transaction to be change by lockOutput address
    uint16_t exChainType = 0; // external chain type. each 15 bit unsign number will be map to a external chain. i.e 0 for btc
    CScript externalPayoutScript;
    uint256 externalTxid;
    uint16_t externalConfirmations = 0;
    uint256 inputsHash; // replay protection

    SERIALIZE_METHODS(CFutureTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.maturity, obj.lockTime, obj.lockOutputIndex, obj.fee,
                  obj.updatableByDestination, obj.exChainType, obj.externalPayoutScript,
                  obj.externalTxid, obj.externalConfirmations, obj.inputsHash);
    }

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("maturity", maturity);
        obj.pushKV("lockTime", (int) lockTime);
        obj.pushKV("lockOutputIndex", (int) lockOutputIndex);
        obj.pushKV("fee", fee);
        obj.pushKV("updatableByDestination", updatableByDestination);
        obj.pushKV("exChainType", exChainType);
        CTxDestination dest;
        if (ExtractDestination(externalPayoutScript, dest)) {
            obj.pushKV("votingAddress", EncodeDestination(dest));
        } else {
            obj.pushKV("externalPayoutAddress", "N/A");
        }
        obj.pushKV("externalTxid", externalTxid.ToString());
        obj.pushKV("externalConfirmations", (int) externalConfirmations);
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

class CNewAssetTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION}; // message version
    std::string name;
    bool isRoot = false;
    std::string rootId;
    bool updatable = true; // If true this asset metadata can be modified using assetTx update process.
    bool isUnique = false; // If true this asset is unique it has an identity per token (NFT flag)
    uint16_t maxMintCount = 0;
    uint8_t decimalPoint = 0;
    std::string referenceHash; // Hash of the underlying physical or digital assets, IPFS hash can be used here.
    uint16_t fee;              // Fee was paid for this asset creation in addition to miner fee. it is a whole non-decimal point value.
    //  distribution
    uint8_t type; // manual, coinbase, address, schedule
    CKeyID targetAddress;
    uint8_t issueFrequency;
    CAmount amount;
    CKeyID ownerAddress;
    CKeyID collateralAddress;
    std::vector<unsigned char> vchSig; //Root asset Signature

    uint16_t exChainType = 0; // External chain type. each 15 bit unsigned number will be map to a external chain. i.e. 0 for btc
    CScript externalPayoutScript;
    uint256 externalTxid;
    uint16_t externalConfirmations = 0;
    uint256 inputsHash; // replay protection

public:

    SERIALIZE_METHODS(CNewAssetTx, obj)
    {
        READWRITE(obj.nVersion, obj.name, obj.updatable, obj.isUnique, obj.maxMintCount,
                  obj.decimalPoint, obj.referenceHash, obj.fee, obj.type, obj.targetAddress,
                  obj.issueFrequency, obj.amount, obj.ownerAddress, obj.collateralAddress);
        READWRITE(obj.isRoot);
        if (!obj.isRoot) {
            //sub asset: serialise the root id and owner signature
            READWRITE(obj.rootId);
            if (!(s.GetType() & SER_GETHASH)) {
                READWRITE(obj.vchSig);
            }
        }
        READWRITE(obj.exChainType, obj.externalPayoutScript, obj.externalTxid,
                  obj.externalConfirmations, obj.inputsHash);
    }

    std::string MakeSignString(CAssetsCache *assetsCache) const;

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("name", name);
        obj.pushKV("isRoot", isRoot);
        if (!isRoot)
            obj.pushKV("rootId", rootId);
        obj.pushKV("isUnique", isUnique);
        obj.pushKV("maxMintCount", maxMintCount);
        obj.pushKV("updatable", updatable);
        obj.pushKV("decimalPoint", (int) decimalPoint);
        obj.pushKV("referenceHash", referenceHash);
        obj.pushKV("fee", fee);
        obj.pushKV("type", type);
        obj.pushKV("targetAddress", EncodeDestination(targetAddress));
        obj.pushKV("ownerAddress", EncodeDestination(ownerAddress));
        if (collateralAddress.IsNull()) {
            obj.pushKV("collateralAddress", "N/A");
        } else {
            obj.pushKV("collateralAddress", EncodeDestination(collateralAddress));
        }
        obj.pushKV("issueFrequency", issueFrequency);
        obj.pushKV("amount", amount);
        obj.pushKV("maxMintCount", maxMintCount);
        obj.pushKV("exChainType", exChainType);
        CTxDestination dest;
        if (ExtractDestination(externalPayoutScript, dest)) {
            obj.pushKV("externalPayoutAddress", EncodeDestination(dest));
        } else {
            obj.pushKV("externalPayoutAddress", "N/A");
        }
        obj.pushKV("externalTxid", externalTxid.ToString());
        obj.pushKV("externalConfirmations", (int) externalConfirmations);
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

class CUpdateAssetTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION}; // message version
    std::string assetId;
    bool updatable = true;     // If true this asset meta can be modified using assetTx update process.
    std::string referenceHash; // Hash of the underlying physical or digital assets, IPFS hash can be used here.
    uint16_t fee;              // Fee was paid for this asset creation in addition to miner fee. It is a whole non-decimal point value.
    //  distribution
    uint8_t type; //manual, coinbase, address, schedule
    CKeyID targetAddress;
    uint8_t issueFrequency;
    uint16_t maxMintCount = 0;
    CAmount amount;
    CKeyID ownerAddress;
    CKeyID collateralAddress;
    std::vector<unsigned char> vchSig; //owner Signature

    uint16_t exChainType = 0; // External chain type. Each 15 bit unsigned number will be map to a external chain. i.e. 0 for btc
    CScript externalPayoutScript;
    uint256 externalTxid;
    uint16_t externalConfirmations = 0;
    uint256 inputsHash; // replay protection

public:
    SERIALIZE_METHODS(CUpdateAssetTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.assetId, obj.updatable, obj.referenceHash, obj.fee,
                  obj.type, obj.targetAddress, obj.issueFrequency, obj.maxMintCount, obj.amount,
                  obj.ownerAddress, obj.collateralAddress, obj.exChainType, obj.externalPayoutScript,
                  obj.externalTxid, obj.externalConfirmations, obj.inputsHash);
        if (!(s.GetType() & SER_GETHASH)) {
            READWRITE(obj.vchSig);
        }
    }

    std::string MakeSignString(CAssetsCache *assetsCache) const;

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("assetId", assetId);
        obj.pushKV("updatable", updatable);
        obj.pushKV("referenceHash", referenceHash);
        obj.pushKV("fee", fee);
        obj.pushKV("type", type);
        obj.pushKV("targetAddress", EncodeDestination(targetAddress));
        obj.pushKV("ownerAddress", EncodeDestination(ownerAddress));
        if (collateralAddress.IsNull()) {
            obj.pushKV("collateralAddress", "N/A");
        } else {
            obj.pushKV("collateralAddress", EncodeDestination(collateralAddress));
        }
        obj.pushKV("issueFrequency", issueFrequency);
        obj.pushKV("amount", amount);
        obj.pushKV("maxMintCount", maxMintCount);
        obj.pushKV("exChainType", exChainType);
        CTxDestination dest;
        if (ExtractDestination(externalPayoutScript, dest)) {
            obj.pushKV("externalPayoutAddress", EncodeDestination(dest));
        } else {
            obj.pushKV("externalPayoutAddress", "N/A");
        }
        obj.pushKV("externalTxid", externalTxid.ToString());
        obj.pushKV("externalConfirmations", (int) externalConfirmations);
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

class CMintAssetTx {
public:
    static const uint16_t CURRENT_VERSION = 1;

    uint16_t nVersion{CURRENT_VERSION}; // message version
    std::string assetId;
    uint16_t fee;
    uint256 inputsHash; // replay protection
    std::vector<unsigned char> vchSig; //owner Signature

public:

    SERIALIZE_METHODS(CMintAssetTx, obj
    )
    {
        READWRITE(obj.nVersion, obj.assetId, obj.fee, obj.inputsHash);
        if (!(s.GetType() & SER_GETHASH)) {
            READWRITE(obj.vchSig);
        }
    }

    std::string MakeSignString(CAssetsCache *assetsCache) const;

    std::string ToString() const;

    void ToJson(UniValue &obj) const {
        obj.clear();
        obj.setObject();
        obj.pushKV("version", nVersion);
        obj.pushKV("assetId", assetId);
        obj.pushKV("fee", fee);
        obj.pushKV("inputsHash", inputsHash.ToString());
    }
};

bool CheckFutureTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state);

bool CheckNewAssetTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state,
                     CAssetsCache *assetsCache);

bool CheckUpdateAssetTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state,
                        const CCoinsViewCache &view, CAssetsCache *assetsCache);

bool CheckMintAssetTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state,
                      const CCoinsViewCache &view, CAssetsCache *assetsCache);

bool CheckProRegTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state,
                   const CCoinsViewCache &view, bool check_sigs);

bool CheckProUpServTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state, bool check_sigs);

bool CheckProUpRegTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state,
                     const CCoinsViewCache &view, bool check_sigs);

bool CheckProUpRevTx(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state, bool check_sigs);

#endif // BITCOIN_EVO_PROVIDERTX_H
