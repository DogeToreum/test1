// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_DOGETOREUMCONSENSUS_H
#define BITCOIN_DOGETOREUMCONSENSUS_H

#include <stdint.h>

#if defined(BUILD_BITCOIN_INTERNAL) && defined(HAVE_CONFIG_H)
#include <config/dogetoreum-config.h>
#if defined(_WIN32)
#if defined(HAVE_DLLEXPORT_ATTRIBUTE)
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL
#endif
#elif defined(HAVE_DEFAULT_VISIBILITY_ATTRIBUTE)
#define EXPORT_SYMBOL __attribute__ ((visibility ("default")))
#endif
#elif defined(MSC_VER) && !defined(STATIC_LIBDOGETOREUMCONSENSUS)
#define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DOGETOREUMCONSENSUS_API_VER 0

typedef enum dogetoreumconsensus_error_t {
    dogetoreumconsensus_ERR_OK = 0,
    dogetoreumconsensus_ERR_TX_INDEX,
    dogetoreumconsensus_ERR_TX_SIZE_MISMATCH,
    dogetoreumconsensus_ERR_TX_DESERIALIZE,
    dogetoreumconsensus_ERR_INVALID_FLAGS,
} dogetoreumconsensus_error;

/** Script verification flags */
enum {
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_NONE = 0,
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_P2SH = (1U << 0), // evaluate P2SH (BIP16) subscripts
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_DERSIG = (1U << 2), // enforce strict DER (BIP66) compliance
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY = (1U << 4), // enforce NULLDUMMY (BIP147)
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9), // enable CHECKLOCKTIMEVERIFY (BIP65)
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10), // enable CHECKSEQUENCEVERIFY (BIP112)
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_ALL =
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_P2SH | dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY | dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
    dogetoreumconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by
/// txTo correctly spends the scriptPubKey pointed to by scriptPubKey under
/// the additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int dogetoreumconsensus_verify_script(const unsigned char *scriptPubKey, unsigned int scriptPubKeyLen,
                                                   const unsigned char *txTo, unsigned int txToLen,
                                                   unsigned int nIn, unsigned int flags, dogetoreumconsensus_error *err);

EXPORT_SYMBOL unsigned int dogetoreumconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // BITCOIN_DOGETOREUMCONSENSUS_H
