// SPDX-FileCopyrightText: 2022 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPFILEENCRYPTION_H
#define QXMPPFILEENCRYPTION_H

#include "QXmppGlobal.h"

#include <memory>

#include <QIODevice>

namespace QCA {
class Cipher;
class Initializer;
}  // namespace QCA

namespace QXmpp::Private::Encryption {

enum Direction {
    Encode,
    Decode,
};

QXMPP_EXPORT QByteArray process(const QByteArray &data, Cipher cipherConfig, Direction direction, const QByteArray &key, const QByteArray &iv);
QXMPP_EXPORT QByteArray generateKey(Cipher cipher);
QXMPP_EXPORT QByteArray generateInitializationVector(Cipher);

// export for tests
class QXMPP_EXPORT EncryptionDevice : public QIODevice
{
public:
    EncryptionDevice(std::unique_ptr<QIODevice> input, Cipher config, const QByteArray &key, const QByteArray &iv);
    ~EncryptionDevice() override;

    bool open(QIODevice::OpenMode mode) override;
    void close() override;
    bool isSequential() const override;
    qint64 size() const override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    bool atEnd() const override;

private:
    Cipher m_cipherConfig;
    bool m_finalized = false;
    std::vector<char> m_outputBuffer;
    std::unique_ptr<QIODevice> m_input;
    std::unique_ptr<QCA::Cipher> m_cipher;
};

class QXMPP_EXPORT DecryptionDevice : public QIODevice
{
public:
    DecryptionDevice(std::unique_ptr<QIODevice> output, Cipher config, const QByteArray &key, const QByteArray &iv);
    ~DecryptionDevice() override;

    bool open(QIODevice::OpenMode mode) override;
    void close() override;
    bool isSequential() const override;
    qint64 size() const override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    Cipher m_cipherConfig;
    std::vector<char> m_outputBuffer;
    std::unique_ptr<QIODevice> m_output;
    std::unique_ptr<QCA::Cipher> m_cipher;
};

}  // namespace QXmpp::Private::Encryption

#endif  // QXMPPFILEENCRYPTION_H
