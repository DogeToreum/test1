// Copyright (c) 2021-2023 The Dogetoreum developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DOGETOREUM_ASSETS_H
#define DOGETOREUM_ASSETS_H

#include <amount.h>
#include <coins.h>
#include <key_io.h>
#include <pubkey.h>
#include <assets/assetstype.h>

class CNewAssetTx;

class CUpdateAssetTx;

class CMintAssetTx;

class UniValue;

struct CAssetOutputEntry;
struct CBlockAssetUndo;

#define MAX_CACHE_ASSETS_SIZE 2500

CAmount getAssetsFeesCoin();

uint16_t getAssetsFees();

bool IsAssetNameValid(std::string name, bool isRoot=false);

bool GetAssetId(const CScript &script, std::string &assetId);

bool validateAmount(const CAmount nAmount, const uint16_t decimalPoint);

bool validateAmount(const std::string &assetId, const CAmount nAmount);

class CAssetMetaData {
public:
    std::string assetId;       //Transaction hash of asset creation
    CAmount circulatingSupply; //update every mint transaction.
    uint16_t mintCount;
    std::string name;
    bool isRoot = false; 
    bool updatable = false; //if true this asset meta can be modify using assetTx update process.
    bool isUnique = false;  //true if this is asset is unique it has an identity per token (NFT flag)
    uint8_t decimalPoint = 0;
    uint16_t maxMintCount;
    std::string referenceHash; //hash of the underlying physical or digital assets, IPFS hash can be used here.
    uint16_t fee;              // fee was paid for this asset creation in addition to miner fee. it is a whole non-decimal point value.
    //  distribution
    uint8_t type; //manual, coinbase, address, schedule
    CKeyID targetAddress;
    uint8_t issueFrequency;
    CAmount amount;
    CKeyID ownerAddress;
    CKeyID collateralAddress;

    CAssetMetaData() {
        SetNull();
    }

    CAssetMetaData(const std::string txid, const CNewAssetTx assetTx);

    SERIALIZE_METHODS(CAssetMetaData, obj
    )
    {
        READWRITE(obj.assetId, obj.circulatingSupply, obj.mintCount, obj.name, obj.isRoot, obj.updatable,
                  obj.isUnique, obj.maxMintCount, obj.decimalPoint, obj.referenceHash, obj.fee,
                  obj.type, obj.targetAddress, obj.issueFrequency, obj.amount, obj.ownerAddress,
                  obj.collateralAddress);
    }

    void SetNull() {
        assetId = "";
        circulatingSupply = CAmount(-1);
        mintCount = uint16_t(-1);
        name = "";
        isRoot = false;
        updatable = false;
        isUnique = false;
        decimalPoint = uint8_t(-1);
        referenceHash = "";
        maxMintCount = uint16_t(-1);
        fee = uint8_t(-1);
        type = uint8_t(-1);
        targetAddress = CKeyID();
        issueFrequency = uint8_t(-1);
        amount = 0;
        ownerAddress = CKeyID();
        collateralAddress = CKeyID();
    }

    void ToJson(UniValue &obj) const;
};

class CDatabaseAssetData {
public:
    CAssetMetaData asset;
    int blockHeight;
    uint256 blockHash;

    CDatabaseAssetData(const CAssetMetaData &asset, const int &nHeight, const uint256 &blockHash);

    CDatabaseAssetData();

    void SetNull() {
        asset.SetNull();
        blockHeight = -1;
        blockHash = uint256();
    }

    bool operator<(const CDatabaseAssetData &rhs) const {
        return asset.assetId < rhs.asset.assetId;
    }

    SERIALIZE_METHODS(CDatabaseAssetData, obj
    )
    {
        READWRITE(obj.asset, obj.blockHeight, obj.blockHash);
    }
};

struct CAssetTransferEntry
{
    CAssetTransfer transfer;
    std::string address;
    COutPoint out;

    CAssetTransferEntry(const CAssetTransfer& transfer, const std::string& address, const COutPoint& out)
    {
        this->transfer = transfer;
        this->address = address;
        this->out = out;
    }

    bool operator<(const CAssetTransferEntry& rhs ) const
    {
        return out < rhs.out;
    }
};

class CAssets {
public:
    std::map <std::string, CDatabaseAssetData> mapAsset;
    std::map <std::string, std::string> mapAssetId;

    std::map <std::pair<std::string, std::string>, CAmount128> mapAssetAddressAmount;

    CAssets(const CAssets &assets) {
        this->mapAsset = assets.mapAsset;
        this->mapAssetId = assets.mapAssetId;
        this->mapAssetAddressAmount = assets.mapAssetAddressAmount;
    }

    CAssets &operator=(const CAssets &other) {
        mapAsset = other.mapAsset;
        mapAssetId = other.mapAssetId;
        mapAssetAddressAmount = other.mapAssetAddressAmount;
        return *this;
    }

    CAssets() {
        SetNull();
    }

    void SetNull() {
        mapAsset.clear();
        mapAssetId.clear();
        mapAssetAddressAmount.clear();
    }
};

class CAssetsCache : public CAssets {
public:
    std::set <CDatabaseAssetData> NewAssetsToRemove;
    std::set <CDatabaseAssetData> NewAssetsToAdd;

    std::set <CAssetTransferEntry> NewAssetsTranferToRemove;
    std::set <CAssetTransferEntry> NewAssetsTransferToAdd;

    CAssetsCache() :
            CAssets() {
        SetNull();
        ClearDirtyCache();
    }

    CAssetsCache(CAssetsCache &cache) :
            CAssets(cache) {
        this->NewAssetsToRemove = cache.NewAssetsToRemove;
        this->NewAssetsToAdd = cache.NewAssetsToAdd;

        this->NewAssetsTranferToRemove = cache.NewAssetsTranferToRemove;
        this->NewAssetsTransferToAdd = cache.NewAssetsTransferToAdd;
    }

    bool InsertAsset(CNewAssetTx newAsset, std::string assetId, int nHeight);

    bool UpdateAsset(CUpdateAssetTx upAsset);

    bool UpdateAsset(std::string assetId, CAmount amount);

    void AddAssetBlance(const CScript &script, const COutPoint &out);
    //undo asset
    bool RemoveAsset(std::string assetId);

    void RemoveAddressBalance(const CScript &script, const COutPoint &out);

    bool UndoUpdateAsset(const CUpdateAssetTx upAsset,
                         const std::vector <std::pair<std::string, CBlockAssetUndo>> &vUndoData);

    bool
    UndoMintAsset(const CMintAssetTx assetTx, const std::vector <std::pair<std::string, CBlockAssetUndo>> &vUndoData);

    bool CheckIfAssetExists(std::string assetId);

    bool GetAssetMetaData(std::string assetId, CAssetMetaData &asset);

    bool GetAssetId(std::string name, std::string &assetId);

    bool Flush();

    bool DumpCacheToDatabase();

    void ClearDirtyCache() {
        NewAssetsToAdd.clear();
        NewAssetsToRemove.clear();

        NewAssetsTransferToAdd.clear();
        NewAssetsTranferToRemove.clear();
    }
};

void AddAssets(const CTransaction &tx, int nHeight, CAssetsCache *assetCache = nullptr,
               std::pair <std::string, CBlockAssetUndo> *undoAssetData = nullptr);

bool GetAssetData(const CScript &script, CAssetOutputEntry &data);

//common function used by tx_verify and wallet
std::vector<std::pair<uint64_t, uint64_t>> combineUniqueIdPairs(const std::vector<std::pair<uint64_t, uint64_t>>& UniqueIds);

#endif //DOGETOREUM_ASSETS_H
