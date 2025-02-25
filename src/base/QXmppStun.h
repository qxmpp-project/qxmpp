// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPSTUN_H
#define QXMPPSTUN_H

#include "QXmppJingleIq.h"
#include "QXmppLogger.h"

#include <QObject>
#include <QSet>

class CandidatePair;
class QDataStream;
class QUdpSocket;
class QTimer;
class QXmppIceComponentPrivate;
class QXmppIceConnectionPrivate;
class QXmppIcePrivate;

///
/// \internal
///
/// The QXmppStunMessage class represents a STUN message.
///
class QXMPP_EXPORT QXmppStunMessage
{
public:
    enum MethodType {
        Binding = 0x1,
        SharedSecret = 0x2,
        Allocate = 0x3,
        Refresh = 0x4,
        Send = 0x6,
        Data = 0x7,
        CreatePermission = 0x8,
        ChannelBind = 0x9
    };

    enum ClassType {
        Request = 0x000,
        Indication = 0x010,
        Response = 0x100,
        Error = 0x110
    };

    QXmppStunMessage();

    quint32 cookie() const;
    void setCookie(quint32 cookie);

    QByteArray id() const;
    void setId(const QByteArray &id);

    quint16 messageClass() const;
    quint16 messageMethod() const;

    quint16 type() const;
    void setType(quint16 type);

    // attributes

    quint32 changeRequest() const;
    void setChangeRequest(quint32 changeRequest);

    quint16 channelNumber() const;
    void setChannelNumber(quint16 channelNumber);

    QByteArray data() const;
    void setData(const QByteArray &data);

    quint32 lifetime() const;
    void setLifetime(quint32 changeRequest);

    QByteArray nonce() const;
    void setNonce(const QByteArray &nonce);

    quint32 priority() const;
    void setPriority(quint32 priority);

    QString realm() const;
    void setRealm(const QString &realm);

    QByteArray reservationToken() const;
    void setReservationToken(const QByteArray &reservationToken);

    quint8 requestedTransport() const;
    void setRequestedTransport(quint8 requestedTransport);

    QString software() const;
    void setSoftware(const QString &software);

    QString username() const;
    void setUsername(const QString &username);

    QByteArray encode(const QByteArray &key = QByteArray(), bool addFingerprint = true) const;
    bool decode(const QByteArray &buffer, const QByteArray &key = QByteArray(), QStringList *errors = nullptr);
    QString toString() const;
    static quint16 peekType(const QByteArray &buffer, quint32 &cookie, QByteArray &id);

    // attributes
    int errorCode;
    QString errorPhrase;
    QByteArray iceControlling;
    QByteArray iceControlled;
    QHostAddress changedHost;
    quint16 changedPort;
    QHostAddress mappedHost;
    quint16 mappedPort;
    QHostAddress otherHost;
    quint16 otherPort;
    QHostAddress sourceHost;
    quint16 sourcePort;
    QHostAddress xorMappedHost;
    quint16 xorMappedPort;
    QHostAddress xorPeerHost;
    quint16 xorPeerPort;
    QHostAddress xorRelayedHost;
    quint16 xorRelayedPort;
    bool useCandidate;

private:
    quint32 m_cookie;
    QByteArray m_id;
    quint16 m_type;

    // attributes
    QSet<quint16> m_attributes;
    quint32 m_changeRequest;
    quint16 m_channelNumber;
    QByteArray m_data;
    quint32 m_lifetime;
    QByteArray m_nonce;
    quint32 m_priority;
    QString m_realm;
    quint8 m_requestedTransport;
    QByteArray m_reservationToken;
    QString m_software;
    QString m_username;
};

///
/// \brief The QXmppIceComponent class represents a piece of a media stream
/// requiring a single transport address, as defined by RFC 5245
/// (Interactive Connectivity Establishment).
///
class QXMPP_EXPORT QXmppIceComponent : public QXmppLoggable
{
    Q_OBJECT

public:
    ~QXmppIceComponent() override;

    int component() const;
    bool isConnected() const;
    QList<QXmppJingleCandidate> localCandidates() const;

    static QList<QHostAddress> discoverAddresses();
    static QList<QUdpSocket *> reservePorts(const QList<QHostAddress> &addresses, int count, QObject *parent = nullptr);

public Q_SLOTS:
    void close();
    void connectToHost();
    qint64 sendDatagram(const QByteArray &datagram);

private Q_SLOTS:
    void checkCandidates();
    void handleDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port);
    void turnConnected();
    void transactionFinished();
    void updateGatheringState();
    void writeStun(const QXmppStunMessage &request);

Q_SIGNALS:
    /// \brief This signal is emitted once ICE negotiation succeeds.
    void connected();

    /// \brief This signal is emitted when a data packet is received.
    void datagramReceived(const QByteArray &datagram);

    /// This signal is emitted when the gathering state of local candidates changes.
    void gatheringStateChanged();

    /// \brief This signal is emitted when the list of local candidates changes.
    void localCandidatesChanged();

private:
    QXmppIceComponent(int component, QXmppIcePrivate *config, QObject *parent = nullptr);

    const std::unique_ptr<QXmppIceComponentPrivate> d;
    friend class QXmppIceComponentPrivate;
    friend class QXmppIceConnection;
};

///
/// \brief The QXmppIceConnection class represents a set of UDP sockets
/// capable of performing Interactive Connectivity Establishment (RFC 5245).
///
/// A typical example is:
///
/// \code
/// QXmppIceConnection *connection = new QXmppIceConnection();
/// connection->setIceControlling(true);
/// connection->addComponent(1);
///
/// // if needed, set STUN / TURN configuration
/// // connection->setStunServer(..);
/// // connection->setTurnServer(..);
///
/// // start listening
/// connection->bind(QXmppIceComponent::discoverAddresses());
///
/// // receive remote information: user, password, candidates
/// // ...
///
/// // set remote information and start connecting
/// connection->setRemoteUser("foo");
/// connection->setRemoteUser("bar");
/// connection->addRemoteCandidate(..);
/// connection->connectToHost();
/// \endcode
///
class QXMPP_EXPORT QXmppIceConnection : public QXmppLoggable
{
    Q_OBJECT

    ///
    /// The ICE gathering state, that is the discovery of local candidates
    ///
    /// \since QXmpp 0.9.3
    ///
    Q_PROPERTY(QXmppIceConnection::GatheringState gatheringState READ gatheringState NOTIFY gatheringStateChanged)

public:
    ///
    /// This enum describes the gathering state of the ICE connection.
    ///
    /// \since QXmpp 0.9.3
    ///
    enum GatheringState {
        NewGatheringState,
        BusyGatheringState,
        CompleteGatheringState
    };
    Q_ENUM(GatheringState)

    QXmppIceConnection(QObject *parent = nullptr);
    ~QXmppIceConnection() override;

    QXmppIceComponent *component(int component);
    void addComponent(int component);
    void setIceControlling(bool controlling);

    QList<QXmppJingleCandidate> localCandidates() const;
    QString localUser() const;
    QString localPassword() const;

    void addRemoteCandidate(const QXmppJingleCandidate &candidate);
    void setRemoteUser(const QString &user);
    void setRemotePassword(const QString &password);

    void setStunServers(const QList<QPair<QHostAddress, quint16>> &servers);
    void setStunServer(const QHostAddress &host, quint16 port = 3478);
    void setTurnServer(const QHostAddress &host, quint16 port = 3478);
    void setTurnUser(const QString &user);
    void setTurnPassword(const QString &password);

    bool bind(const QList<QHostAddress> &addresses);
    bool isConnected() const;

    // documentation needs to be here, see https://stackoverflow.com/questions/49192523/
    ///
    /// Returns the ICE gathering state, that is the discovery of local
    /// candidates.
    ///
    /// \since QXmpp 0.9.3
    ///
    GatheringState gatheringState() const;

Q_SIGNALS:
    /// \brief This signal is emitted once ICE negotiation succeeds.
    void connected();

    /// \brief This signal is emitted when ICE negotiation fails.
    void disconnected();

    ///
    /// \brief This signal is emitted when the gathering state of local candidates changes.
    ///
    /// \since QXmpp 0.9.3
    ///
    void gatheringStateChanged();

    /// \brief This signal is emitted when the list of local candidates changes.
    void localCandidatesChanged();

public Q_SLOTS:
    void close();
    void connectToHost();

private Q_SLOTS:
    void slotConnected();
    void slotGatheringStateChanged();
    void slotTimeout();

private:
    const std::unique_ptr<QXmppIceConnectionPrivate> d;
};

#endif
