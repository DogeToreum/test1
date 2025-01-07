// Copyright (c) 2025 The Dogetoreum developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef DOGETOREUM_UPLOAD_DOWNLOAD_H
#define DOGETOREUM_UPLOAD_DOWNLOAD_H

#include <util/system.h>

#include <iostream>
#include <vector>
#include <string>
#include <QWidget>

static std::string GET_URI = "/get/";
static std::string UPLOAD_URI = "/upload";
static std::string DEFAULT_IPFS_SERVICE_URL = "ipfsm.dogetoreum.com";
static std::string DEFAULT_IPFS_GATEWAY_URL = "https://ipfsweb.dogetoreum.com/ipfs/";

void download(const std::string cid, std::string& response_data);
void upload(const std::string& file_path, std::string& response_data);
void pickAndUploadFileForIpfs(QWidget *qWidget, std::string& cid);

#endif //DOGETOREUM_UPLOAD_DOWNLOAD_H
