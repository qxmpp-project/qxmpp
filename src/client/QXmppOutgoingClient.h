// SPDX-FileCopyrightText: 2010 Manjeet Dahiya <manjeetdahiya@gmail.com>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOUTGOINGCLIENT_H
#define QXMPPOUTGOINGCLIENT_H

#include "QXmppClient.h"
#include "QXmppStanza.h"
#include "QXmppStream.h"

class QDomElement;
class QSslError;

class QXmppConfiguration;
class QXmppPresence;
class QXmppIq;
class QXmppMessage;
class QXmppStreamFeatures;

class QXmppOutgoingClient;
class QXmppOutgoingClientPrivate;

namespace QXmpp::Private {
class PingManager;
class C2sStreamManager;
}  // namespace QXmpp::Private

///
/// \brief The QXmppOutgoingClient class represents an outgoing XMPP stream
/// to an XMPP server.
///
class QXMPP_EXPORT QXmppOutgoingClient : public QXmppStream
{
    Q_OBJECT

public:
    QXmppOutgoingClient(QObject *parent);
    ~QXmppOutgoingClient() override;

    void connectToHost();
    bool isAuthenticated() const;
    bool isConnected() const override;
    bool isClientStateIndicationEnabled() const;
    QXmppTask<QXmpp::Private::IqResult> sendIq(QXmppIq &&);

    /// Returns the used socket
    QSslSocket *socket() const { return QXmppStream::socket(); };
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppConfiguration &configuration();

    QXmpp::Private::C2sStreamManager &c2sStreamManager();

Q_SIGNALS:
    /// This signal is emitted when an error is encountered.
    void error(QXmppClient::Error);

    /// This signal is emitted when an element is received.
    void elementReceived(const QDomElement &element, bool &handled);

    /// This signal is emitted when a presence is received.
    void presenceReceived(const QXmppPresence &);

    /// This signal is emitted when a message is received.
    void messageReceived(const QXmppMessage &);

    /// This signal is emitted when an IQ response (type result or error) has
    /// been received that was not handled by elementReceived().
    void iqReceived(const QXmppIq &);

    /// This signal is emitted when SSL errors are encountered.
    void sslErrors(const QList<QSslError> &errors);

protected:
    /// \cond
    // Overridable methods
    void handleStart() override;
    void handleStanza(const QDomElement &element) override;
    void handleStream(const QDomElement &element) override;
    /// \endcond

public Q_SLOTS:
    void disconnectFromHost() override;

private Q_SLOTS:
    void _q_socketDisconnected();
    void socketError(QAbstractSocket::SocketError);
    void socketSslErrors(const QList<QSslError> &);

private:
    void onSMResumeFinished();
    void onSMEnableFinished();
    void throwKeepAliveError();

    friend class QXmppOutgoingClientPrivate;
    friend class QXmpp::Private::PingManager;
    friend class QXmpp::Private::C2sStreamManager;

    const std::unique_ptr<QXmppOutgoingClientPrivate> d;
};

namespace QXmpp::Private {

class C2sStreamManager
{
public:
    explicit C2sStreamManager(QXmppOutgoingClient *q);

    bool handleElement(const QDomElement &);
    bool hasResumeAddress() const { return m_canResume && !m_resumeHost.isEmpty() && m_resumePort; }
    std::pair<QString, quint16> resumeAddress() const { return { m_resumeHost, m_resumePort }; }
    void onStreamStart();
    void onStreamFeatures(const QXmppStreamFeatures &);
    void onDisconnecting();
    bool canResume() const { return m_canResume; }
    bool enabled() const { return m_enabled; }
    bool streamResumed() const { return m_streamResumed; }
    bool canRequestResume() const { return m_smAvailable && m_canResume; }
    void requestResume();
    bool canRequestEnable() const { return m_smAvailable; }
    void requestEnable();

private:
    bool setResumeAddress(const QString &address);

    QXmppOutgoingClient *q;

    bool m_smAvailable = false;
    QString m_smId;
    bool m_canResume = false;
    bool m_isResuming = false;
    QString m_resumeHost;
    quint16 m_resumePort = 0;
    bool m_enabled = false;
    bool m_streamResumed = false;
};

}  // namespace QXmpp::Private

#endif  // QXMPPOUTGOINGCLIENT_H
