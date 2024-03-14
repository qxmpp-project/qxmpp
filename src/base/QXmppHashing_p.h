// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPHASHING_H
#define QXMPPHASHING_H

#include "QXmppError.h"
#include "QXmppGlobal.h"
#include "QXmppHash.h"

#include <memory>
#include <variant>
#include <vector>

#include <QCryptographicHash>

template<typename T>
class QFuture;
class QXmppHash;

namespace QXmpp::Private {

struct HashingResult {
    using Result = std::variant<std::vector<QXmppHash>, Cancelled, QXmppError>;

    HashingResult(Result result, std::unique_ptr<QIODevice> data)
        : result(std::move(result)), data(std::move(data))
    {
    }

    Result result;
    std::unique_ptr<QIODevice> data;
};

struct HashVerificationResult {
    struct NoStrongHashes { };
    struct NotMatching { };
    struct Verified { };
    using Result = std::variant<NoStrongHashes, NotMatching, Verified, Cancelled, QXmppError>;

    HashVerificationResult(Result result, std::unique_ptr<QIODevice> data)
        : result(std::move(result)), data(std::move(data))
    {
    }

    Result result;
    std::unique_ptr<QIODevice> data;
};

using HashingResultPtr = std::shared_ptr<HashingResult>;
using HashVerificationResultPtr = std::shared_ptr<HashVerificationResult>;

bool isHashingAlgorithmSecure(HashAlgorithm algorithm);
uint16_t hashPriority(HashAlgorithm algorithm);

// QXMPP_EXPORT for unit tests
QXMPP_EXPORT QFuture<HashingResultPtr> calculateHashes(std::unique_ptr<QIODevice> data, std::vector<HashAlgorithm> hashes);
QFuture<HashVerificationResultPtr> verifyHashes(std::unique_ptr<QIODevice> data, std::vector<QXmppHash> hashes);

}  // namespace QXmpp::Private

#endif  // QXMPPHASHING_H
