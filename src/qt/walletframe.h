// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2020-2023 The Dogetoreum developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_WALLETFRAME_H
#define BITCOIN_QT_WALLETFRAME_H

#include <QFrame>
#include <QMap>

class BitcoinGUI;

class ClientModel;

class SendCoinsRecipient;

class WalletModel;

class WalletView;

class SendFuturesRecipient;

class SmartnodeList;

QT_BEGIN_NAMESPACE
class QStackedWidget;
QT_END_NAMESPACE

/**
 * A container for embedding all wallet-related
 * controls into BitcoinGUI. The purpose of this class is to allow future
 * refinements of the wallet controls with minimal need for further
 * modifications to BitcoinGUI, thus greatly simplifying merges while
 * reducing the risk of breaking top-level stuff.
 */
class WalletFrame : public QFrame {
    Q_OBJECT

public:
    explicit WalletFrame(BitcoinGUI *_gui = nullptr);

    ~WalletFrame();

    void setClientModel(ClientModel *clientModel);

    bool addWallet(WalletModel *walletModel);

    void setCurrentWallet(WalletModel *wallet_model);

    void removeWallet(WalletModel *wallet_model);

    void removeAllWallets();

    bool handlePaymentRequest(const SendCoinsRecipient &recipient);

    void showOutOfSyncWarning(bool fShow);

    Q_SIGNALS:
            /** Notify that the user has requested more information about the out-of-sync warning */
            void requestedSyncWarningInfo();

private:
    QStackedWidget *walletStack;
    BitcoinGUI *gui;
    ClientModel *clientModel;
    QMap<WalletModel *, WalletView *> mapWalletViews;
    SmartnodeList *smartnodeListPage;

    bool bOutOfSync;

public:
    WalletView *currentWalletView() const;

    WalletModel *currentWalletModel() const;

public
    Q_SLOTS:
            /** Switch to overview (home) page */
            void gotoOverviewPage();

    /** Switch to history (transactions) page */
    void gotoHistoryPage();

    /** Switch to smartnode page */
    void gotoSmartnodePage();

    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();

    /** Switch to send coins page */
    void gotoSendCoinsPage(QString addr = "");

    /** Switch to send assets page */
    void gotoSendAssetsPage(QString addr = "");

    /** Switch to create assets page */
    void gotoCreateAssetsPage();

    /** Switch to create assets page */
    void gotoUpdateAssetsPage();

    /** Switch to my assets page */
    void gotoMyAssetsPage();
    
    /** Switch to CoinJoin coins page */
    void gotoCoinJoinCoinsPage(QString addr = "");

    /** Show Sign/Verify Message dialog and switch to sign message tab */
    void gotoSignMessageTab(QString addr = "");

    /** Show Sign/Verify Message dialog and switch to verify message tab */
    void gotoVerifyMessageTab(QString addr = "");

    /** Encrypt the wallet */
    void encryptWallet();

    /** Backup the wallet */
    void backupWallet();

    /** Change encrypted wallet passphrase */
    void changePassphrase();

    /** Ask for passphrase to unlock wallet temporarily */
    void unlockWallet();

    /** Lock wallet */
    void lockWallet();

    /** Show used sending addresses */
    void usedSendingAddresses();

    /** Show used receiving addresses */
    void usedReceivingAddresses();

    /** Pass on signal over requested out-of-sync-warning information */
    void outOfSyncWarningClicked();
};

#endif // BITCOIN_QT_WALLETFRAME_H
