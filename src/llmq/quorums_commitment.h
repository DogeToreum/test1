// Copyright (c) 2018-2021 The Dash Core developers
// Copyright (c) 2020-2023 The Dogetoreum developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_LLMQ_QUORUMS_COMMITMENT_H
#define BITCOIN_LLMQ_QUORUMS_COMMITMENT_H

#include <llmq/quorums_utils.h>
#include <bls/bls.h>
#include <univalue.h>

class CValidationState;

namespace llmq {

// This message is an aggregation of all received premature commitments and only valid if
// enough (>=threshold) premature commitments were aggregated
// This is mined on-chain as part of TRANSACTION_QUORUM_COMMITMENT
    class CFinalCommitment {
    public:
        static const uint16_t CURRENT_VERSION = 2;

        uint16_t nVersion{1};
        Consensus::LLMQType llmqType{Consensus::LLMQ_NONE};
        uint256 quorumHash;
        std::vector<bool> signers;
        std::vector<bool> validMembers;
        Consensus::CQuorumUpdateVoteVec quorumUpdateVotes;

        CBLSPublicKey quorumPublicKey;
        uint256 quorumVvecHash;

        CBLSSignature quorumSig; // recovered threshold sig of blockHash+validMembers+pubKeyHash+vvecHash
        CBLSSignature membersSig; // aggregated member sig of blockHash+validMembers+pubKeyHash+vvecHash

        CFinalCommitment() = default;

        CFinalCommitment(const Consensus::LLMQParams &params, const uint256 &_quorumHash);

        int CountSigners() const {
            return int(std::count(signers.begin(), signers.end(), true));
        }

        int CountValidMembers() const {
            return int(std::count(validMembers.begin(), validMembers.end(), true));
        }

        bool Verify(const CBlockIndex *pQuorumBaseBlockIndex, bool checkSigs) const;

        bool VerifyNull() const;

        bool VerifySizes(const Consensus::LLMQParams &params) const;

        SERIALIZE_METHODS(CFinalCommitment, obj)
        {
            READWRITE(obj.nVersion, obj.llmqType, obj.quorumHash, DYNBITSET(obj.signers), DYNBITSET(obj.validMembers));
            if (obj.nVersion > 1)
            {
               READWRITE(obj.quorumUpdateVotes);
            }
            READWRITE(obj.quorumPublicKey, obj.quorumVvecHash, obj.quorumSig, obj.membersSig);
        }

        bool IsNull() const {
            if (std::count(signers.begin(), signers.end(), true) ||
                std::count(validMembers.begin(), validMembers.end(), true)) {
                return false;
            }
            if (quorumPublicKey.IsValid() ||
                !quorumVvecHash.IsNull() ||
                membersSig.IsValid() ||
                quorumSig.IsValid()) {
                return false;
            }
            return true;
        }

        void ToJson(UniValue &obj) const {
            obj.setObject();
            obj.pushKV("version", int(nVersion));
            obj.pushKV("llmqType", int(llmqType));
            obj.pushKV("quorumHash", quorumHash.ToString());
            obj.pushKV("signersCount", CountSigners());
            obj.pushKV("signers", CLLMQUtils::ToHexStr(signers));
            obj.pushKV("validMembersCount", CountValidMembers());
            obj.pushKV("validMembers", CLLMQUtils::ToHexStr(validMembers));
            obj.pushKV("quorumPublicKey", quorumPublicKey.ToString());
            obj.pushKV("quorumVvecHash", quorumVvecHash.ToString());
            if (quorumUpdateVotes.size() > 0) {
                UniValue votes(UniValue::VARR);
                for (const auto& vote: quorumUpdateVotes) {
                    UniValue eObj(UniValue::VOBJ);
                    eObj.pushKV("bit", vote.bit);
                    eObj.pushKV("votes", vote.votes);
                    votes.push_back(eObj);
                }
                obj.pushKV("updateVotes", votes);
            }
            obj.pushKV("quorumSig", quorumSig.ToString());
            obj.pushKV("membersSig", membersSig.ToString());
        }
    };

    using CFinalCommitmentPtr = std::unique_ptr<CFinalCommitment>;

    class CFinalCommitmentTxPayload {
    public:
        static const uint16_t CURRENT_VERSION = 1;

        uint16_t nVersion{CURRENT_VERSION};
        uint32_t nHeight{(uint32_t) - 1};
        CFinalCommitment commitment;

        SERIALIZE_METHODS(CFinalCommitmentTxPayload, obj)
        {
            READWRITE(obj.nVersion, obj.nHeight, obj.commitment);
        }

        void ToJson(UniValue &obj) const {
            obj.setObject();
            obj.pushKV("version", int(nVersion));
            obj.pushKV("height", int(nHeight));

            UniValue qcObj;
            commitment.ToJson(qcObj);
            obj.pushKV("commitment", qcObj);
        }
    };

    bool CheckLLMQCommitment(const CTransaction &tx, const CBlockIndex *pindexPrev, CValidationState &state);

} // namespace llmq

#endif // BITCOIN_LLMQ_QUORUMS_COMMITMENT_H
