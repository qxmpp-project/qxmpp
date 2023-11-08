// SPDX-FileCopyrightText: 2009 Manjeet Dahiya <manjeetdahiya@gmail.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPCLIENT_H
#define QXMPPCLIENT_H

#include "QXmppConfiguration.h"
#include "QXmppLogger.h"
#include "QXmppPresence.h"
#include "QXmppSendResult.h"
#include "QXmppSendStanzaParams.h"

#include <memory>
#include <variant>

#include <QAbstractSocket>
#include <QObject>
#include <QSslError>

template<typename T>
class QXmppTask;

class QXmppE2eeExtension;
class QXmppClientExtension;
class QXmppClientPrivate;
class QXmppPresence;
class QXmppMessage;
class QXmppIq;
class QXmppStream;
class QXmppInternalClientExtension;

// managers
class QXmppDiscoveryIq;
class QXmppRosterManager;
class QXmppVCardManager;
class QXmppVersionManager;

///
/// \defgroup Core Core classes
///
/// Core classes include all necessary classes to build a basic client or
/// server application. This for example also includes the logging class
/// QXmppLogger.
///

///
/// \defgroup Managers Managers
///
/// Managers are used to extend the basic QXmppClient. Some of them are loaded
/// by default, others need to be added to the client using
/// QXmppClient::addExtension().
///

///
/// \brief The QXmppClient class is the main class for using QXmpp.
///
/// It provides the user all the required functionality to connect to the
/// server and perform operations afterwards.
///
/// This class will provide the handle/reference to QXmppRosterManager
/// (roster management), QXmppVCardManager (vCard manager), and
/// QXmppVersionManager (software version information).
///
/// By default, the client will automatically try reconnecting to the server.
/// You can change that behaviour using
/// QXmppConfiguration::setAutoReconnectionEnabled().
///
/// Not all the managers or extensions have been enabled by default. One can
/// enable/disable the managers using the functions \c addExtension() and
/// \c removeExtension(). \c findExtension() can be used to find a
/// reference/pointer to a particular instantiated and enabled manager.
///
/// List of managers enabled by default:
/// - QXmppRosterManager
/// - QXmppVCardManager
/// - QXmppVersionManager
/// - QXmppDiscoveryManager
/// - QXmppEntityTimeManager
///
/// \ingroup Core
///
class QXMPP_EXPORT QXmppClient : public QXmppLoggable
{
    Q_OBJECT

    /// The QXmppLogger associated with the current QXmppClient
    Q_PROPERTY(QXmppLogger *logger READ logger WRITE setLogger NOTIFY loggerChanged)
    /// The client's current state
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    using IqResult = std::variant<QDomElement, QXmppError>;
    using EmptyResult = std::variant<QXmpp::Success, QXmppError>;

    /// An enumeration for type of error.
    /// Error could come due a TCP socket or XML stream or due to various stanzas.
    enum Error {
        NoError,          ///< No error.
        SocketError,      ///< Error due to TCP socket.
        KeepAliveError,   ///< Error due to no response to a keep alive.
        XmppStreamError,  ///< Error due to XML stream.
    };
    Q_ENUM(Error)

    /// This enumeration describes a client state.
    enum State {
        DisconnectedState,  ///< Disconnected from the server.
        ConnectingState,    ///< Trying to connect to the server.
        ConnectedState      ///< Connected to the server.
    };
    Q_ENUM(State)

    /// Describes the use of \xep{0198}: Stream Management
    enum StreamManagementState {
        /// Stream Management is not used.
        NoStreamManagement,
        /// Stream Management is used and the previous stream has not been resumed.
        NewStream,
        /// Stream Management is used and the previous stream has been resumed.
        ResumedStream
    };

    /// Used to decide which extensions should be added by default.
    /// \since QXmpp 1.6
    enum InitialExtensions {
        /// Creates a client without any extensions.
        NoExtensions,
        /// Creates a client with the default set of extensions.
        BasicExtensions,
    };

    QXmppClient(InitialExtensions, QObject *parent = nullptr);
    QXmppClient(QObject *parent = nullptr);
    ~QXmppClient() override;

    bool addExtension(QXmppClientExtension *extension);
    template<typename T, typename... Args>
    T *addNewExtension(Args... args)
    {
        // it's impossible that addExtension() returns false: ext is a new object
        auto *ext = new T(args...);
        addExtension(ext);
        return ext;
    }
    bool insertExtension(int index, QXmppClientExtension *extension);
    bool removeExtension(QXmppClientExtension *extension);
    QXmppE2eeExtension *encryptionExtension() const;
    void setEncryptionExtension(QXmppE2eeExtension *);

    QList<QXmppClientExtension *> extensions();

    ///
    /// \brief Returns the extension which can be cast into type T*, or 0
    /// if there is no such extension.
    ///
    /// Usage example:
    /// \code
    /// QXmppDiscoveryManager* ext = client->findExtension<QXmppDiscoveryManager>();
    /// if(ext)
    /// {
    ///     //extension found, do stuff...
    /// }
    /// \endcode
    ///
    template<typename T>
    T *findExtension()
    {
        const QList<QXmppClientExtension *> list = extensions();
        for (auto ext : list) {
            T *extension = qobject_cast<T *>(ext);
            if (extension) {
                return extension;
            }
        }
        return nullptr;
    }

    ///
    /// \brief Returns the index of an extension
    ///
    /// Usage example:
    /// \code
    /// int index = client->indexOfExtension<QXmppDiscoveryManager>();
    /// if (index > 0) {
    ///     // extension found, do stuff...
    /// } else {
    ///     // extension not found
    /// }
    /// \endcode
    ///
    /// \since QXmpp 1.2
    ///
    template<typename T>
    int indexOfExtension()
    {
        auto list = extensions();
        for (int i = 0; i < list.size(); ++i) {
            if (qobject_cast<T *>(list.at(i)) != nullptr) {
                return i;
            }
        }
        return -1;
    }

    bool isAuthenticated() const;
    bool isConnected() const;

    bool isActive() const;
    void setActive(bool active);

    StreamManagementState streamManagementState() const;

    QXmppPresence clientPresence() const;
    void setClientPresence(const QXmppPresence &presence);

    QXmppConfiguration &configuration();

    // documentation needs to be here, see https://stackoverflow.com/questions/49192523/
    /// Returns the QXmppLogger associated with the current QXmppClient.
    QXmppLogger *logger() const;
    void setLogger(QXmppLogger *logger);

    QAbstractSocket::SocketError socketError();
    QString socketErrorString() const;

    // documentation needs to be here, see https://stackoverflow.com/questions/49192523/
    /// Returns the client's current state.
    State state() const;
    QXmppStanza::Error::Condition xmppStreamError();

    QXmppTask<QXmpp::SendResult> sendSensitive(QXmppStanza &&, const std::optional<QXmppSendStanzaParams> & = {});
    QXmppTask<QXmpp::SendResult> send(QXmppStanza &&, const std::optional<QXmppSendStanzaParams> & = {});
    QXmppTask<QXmpp::SendResult> reply(QXmppStanza &&stanza, const std::optional<QXmppE2eeMetadata> &e2eeMetadata, const std::optional<QXmppSendStanzaParams> & = {});
    QXmppTask<IqResult> sendIq(QXmppIq &&, const std::optional<QXmppSendStanzaParams> & = {});
    QXmppTask<IqResult> sendSensitiveIq(QXmppIq &&, const std::optional<QXmppSendStanzaParams> & = {});
    QXmppTask<EmptyResult> sendGenericIq(QXmppIq &&, const std::optional<QXmppSendStanzaParams> & = {});

#if QXMPP_DEPRECATED_SINCE(1, 1)
    QT_DEPRECATED_X("Use QXmppClient::findExtension<QXmppRosterManager>() instead")
    QXmppRosterManager &rosterManager();

    QT_DEPRECATED_X("Use QXmppClient::findExtension<QXmppVCardManager>() instead")
    QXmppVCardManager &vCardManager();

    QT_DEPRECATED_X("Use QXmppClient::findExtension<QXmppVersionManager>() instead")
    QXmppVersionManager &versionManager();
#endif

Q_SIGNALS:

    /// This signal is emitted when the client connects successfully to the
    /// XMPP server i.e. when a successful XMPP connection is established.
    /// XMPP Connection involves following sequential steps:
    ///     - TCP socket connection
    ///     - Client sends start stream
    ///     - Server sends start stream
    ///     - TLS negotiation (encryption)
    ///     - Authentication
    ///     - Resource binding
    ///     - Session establishment
    ///
    /// After all these steps a successful XMPP connection is established and
    /// connected() signal is emitted.
    ///
    /// After the connected() signal is emitted QXmpp will send the roster
    /// request to the server. On receiving the roster, QXmpp will emit
    /// QXmppRosterManager::rosterReceived(). After this signal,
    /// QXmppRosterManager object gets populated and you can use
    /// \c findExtension<QXmppRosterManager>() to get the handle of
    /// QXmppRosterManager object.
    void connected();

    /// This signal is emitted when the XMPP connection disconnects.
    void disconnected();

    /// This signal is emitted when the XMPP connection encounters any error.
    /// The QXmppClient::Error parameter specifies the type of error occurred.
    /// It could be due to TCP socket or the xml stream or the stanza.
    /// Depending upon the type of error occurred use the respective get function to
    /// know the error.
    void error(QXmppClient::Error);

    /// This signal is emitted when the logger changes.
    void loggerChanged(QXmppLogger *logger);

    /// Notifies that an XMPP message stanza is received. The QXmppMessage
    /// parameter contains the details of the message sent to this client.
    /// In other words whenever someone sends you a message this signal is
    /// emitted.
    void messageReceived(const QXmppMessage &message);

    /// Notifies that an XMPP presence stanza is received. The QXmppPresence
    /// parameter contains the details of the presence sent to this client.
    /// This signal is emitted when someone login/logout or when someone's status
    /// changes Busy, Idle, Invisible etc.
    void presenceReceived(const QXmppPresence &presence);

    /// This signal is emitted when IQs of type result or error are received by
    /// the client and no registered QXmppClientExtension could handle it.
    ///
    /// This is useful when it is only important to check whether the response
    /// of an IQ was successful. However, the recommended way is still to use an
    /// additional QXmppClientExtension for this kind of tasks.
    void iqReceived(const QXmppIq &iq);

    /// This signal is emitted to indicate that one or more SSL errors were
    /// encountered while establishing the identity of the server.
    void sslErrors(const QList<QSslError> &errors);

    /// This signal is emitted when the client state changes.
    void stateChanged(QXmppClient::State state);

public Q_SLOTS:
    void connectToServer(const QXmppConfiguration &,
                         const QXmppPresence &initialPresence =
                             QXmppPresence());
    void connectToServer(const QString &jid,
                         const QString &password);
    void disconnectFromServer();
    bool sendPacket(const QXmppNonza &);
    void sendMessage(const QString &bareJid, const QString &message);

private:
    void injectIq(const QDomElement &element, const std::optional<QXmppE2eeMetadata> &e2eeMetadata);
    bool injectMessage(QXmppMessage &&message);

private Q_SLOTS:
    void _q_elementReceived(const QDomElement &element, bool &handled);
    void _q_reconnect();
    void _q_socketStateChanged(QAbstractSocket::SocketState state);
    void _q_streamConnected();
    void _q_streamDisconnected();
    void _q_streamError(QXmppClient::Error error);

private:
    const std::unique_ptr<QXmppClientPrivate> d;

    friend class QXmppClientExtension;
    friend class QXmppInternalClientExtension;
    friend class TestClient;
};

#endif  // QXMPPCLIENT_H
