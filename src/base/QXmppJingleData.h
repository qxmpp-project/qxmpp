// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
// SPDX-FileCopyrightText: 2023 Tibor Csötönyi <work@taibsu.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPJINGLEIQ_H
#define QXMPPJINGLEIQ_H

#include "QXmppIq.h"

#include <variant>

#include <QHostAddress>

class QXmppJingleCandidatePrivate;
class QXmppJingleDescriptionPrivate;
class QXmppJingleIqContentPrivate;
class QXmppJingleIqReasonPrivate;
class QXmppJingleIqPrivate;
class QXmppJinglePayloadTypePrivate;
class QXmppJingleRtpCryptoElementPrivate;
class QXmppJingleRtpEncryptionPrivate;
class QXmppJingleRtpFeedbackPropertyPrivate;
class QXmppJingleRtpHeaderExtensionPropertyPrivate;
class QXmppSdpParameterPrivate;
class QXmppJingleMessageInitiationElementPrivate;
class QXmppCallInviteElementPrivate;

class QXMPP_EXPORT QXmppSdpParameter
{
public:
    QXmppSdpParameter();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppSdpParameter)

    QString name() const;
    void setName(const QString &name);

    QString value() const;
    void setValue(const QString &value);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isSdpParameter(const QDomElement &element);

private:
    QSharedDataPointer<QXmppSdpParameterPrivate> d;
};

class QXMPP_EXPORT QXmppJingleRtpCryptoElement
{
public:
    QXmppJingleRtpCryptoElement();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleRtpCryptoElement)

    uint32_t tag() const;
    void setTag(uint32_t tag);

    QString cryptoSuite() const;
    void setCryptoSuite(const QString &cryptoSuite);

    QString keyParams() const;
    void setKeyParams(const QString &keyParams);

    QString sessionParams() const;
    void setSessionParams(const QString &sessionParams);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isJingleRtpCryptoElement(const QDomElement &element);

private:
    QSharedDataPointer<QXmppJingleRtpCryptoElementPrivate> d;
};

class QXMPP_EXPORT QXmppJingleRtpEncryption
{
public:
    QXmppJingleRtpEncryption();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleRtpEncryption)

    bool isRequired() const;
    void setRequired(bool isRequired);

    QVector<QXmppJingleRtpCryptoElement> cryptoElements() const;
    void setCryptoElements(const QVector<QXmppJingleRtpCryptoElement> &cryptoElements);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isJingleRtpEncryption(const QDomElement &element);

private:
    QSharedDataPointer<QXmppJingleRtpEncryptionPrivate> d;
};

class QXMPP_EXPORT QXmppJingleRtpFeedbackProperty
{
public:
    QXmppJingleRtpFeedbackProperty();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleRtpFeedbackProperty)

    QString type() const;
    void setType(const QString &type);

    QString subtype() const;
    void setSubtype(const QString &subtype);

    QVector<QXmppSdpParameter> parameters() const;
    void setParameters(const QVector<QXmppSdpParameter> &parameters);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isJingleRtpFeedbackProperty(const QDomElement &element);

private:
    QSharedDataPointer<QXmppJingleRtpFeedbackPropertyPrivate> d;
};

class QXMPP_EXPORT QXmppJingleRtpFeedbackInterval
{
public:
    QXmppJingleRtpFeedbackInterval();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleRtpFeedbackInterval)

    uint64_t value() const;
    void setValue(uint64_t value);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isJingleRtpFeedbackInterval(const QDomElement &element);

private:
    uint64_t m_value;
};

class QXMPP_EXPORT QXmppJingleRtpHeaderExtensionProperty
{
public:
    enum Senders {
        /// The initiator and the sender are allowed.
        Both,
        /// Only the initiator is allowed.
        Initiator,
        /// Only the responder is allowed.
        Responder
    };

    QXmppJingleRtpHeaderExtensionProperty();

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleRtpHeaderExtensionProperty)

    uint32_t id() const;
    void setId(uint32_t id);

    QString uri() const;
    void setUri(const QString &uri);

    Senders senders() const;
    void setSenders(Senders senders);

    QVector<QXmppSdpParameter> parameters() const;
    void setParameters(const QVector<QXmppSdpParameter> &parameters);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isJingleRtpHeaderExtensionProperty(const QDomElement &element);

private:
    QSharedDataPointer<QXmppJingleRtpHeaderExtensionPropertyPrivate> d;
};

///
/// \brief The QXmppJinglePayloadType class represents a payload type
/// as specified by \xep{0167}: Jingle RTP Sessions and RFC 5245.
///
class QXMPP_EXPORT QXmppJinglePayloadType
{
public:
    QXmppJinglePayloadType();
    QXmppJinglePayloadType(const QXmppJinglePayloadType &other);
    ~QXmppJinglePayloadType();

    unsigned char channels() const;
    void setChannels(unsigned char channels);

    unsigned int clockrate() const;
    void setClockrate(unsigned int clockrate);

    unsigned char id() const;
    void setId(unsigned char id);

    unsigned int maxptime() const;
    void setMaxptime(unsigned int maxptime);

    QString name() const;
    void setName(const QString &name);

    QMap<QString, QString> parameters() const;
    void setParameters(const QMap<QString, QString> &parameters);

    unsigned int ptime() const;
    void setPtime(unsigned int ptime);

    QVector<QXmppJingleRtpFeedbackProperty> rtpFeedbackProperties() const;
    void setRtpFeedbackProperties(const QVector<QXmppJingleRtpFeedbackProperty> &rtpFeedbackProperties);

    QVector<QXmppJingleRtpFeedbackInterval> rtpFeedbackIntervals() const;
    void setRtpFeedbackIntervals(const QVector<QXmppJingleRtpFeedbackInterval> &rtpFeedbackIntervals);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    QXmppJinglePayloadType &operator=(const QXmppJinglePayloadType &other);
    bool operator==(const QXmppJinglePayloadType &other) const;

private:
    QSharedDataPointer<QXmppJinglePayloadTypePrivate> d;
};

class QXMPP_EXPORT QXmppJingleDescription
{
public:
    QXmppJingleDescription();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleDescription)

    QString media() const;
    void setMedia(const QString &media);

    quint32 ssrc() const;
    void setSsrc(quint32 ssrc);

    QString type() const;
    void setType(const QString &type);

    void addPayloadType(const QXmppJinglePayloadType &payload);
    const QList<QXmppJinglePayloadType> &payloadTypes() const;
    void setPayloadTypes(const QList<QXmppJinglePayloadType> &payloadTypes);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppJingleDescriptionPrivate> d;
};

///
/// \brief The QXmppJingleCandidate class represents a transport candidate
/// as specified by \xep{0176}: Jingle ICE-UDP Transport Method.
///
class QXMPP_EXPORT QXmppJingleCandidate
{
public:
    /// This enum is used to describe a candidate's type.
    enum Type {
        HostType,             ///< Host candidate, a local address/port.
        PeerReflexiveType,    ///< Peer-reflexive candidate,
                              ///< the address/port as seen from the peer.
        ServerReflexiveType,  ///< Server-reflexive candidate,
                              ///< the address/port as seen by the STUN server
        RelayedType           ///< Relayed candidate, a candidate from
                              ///< a TURN relay.
    };

    QXmppJingleCandidate();
    QXmppJingleCandidate(const QXmppJingleCandidate &other);
    QXmppJingleCandidate(QXmppJingleCandidate &&);
    ~QXmppJingleCandidate();

    QXmppJingleCandidate &operator=(const QXmppJingleCandidate &other);
    QXmppJingleCandidate &operator=(QXmppJingleCandidate &&);

    int component() const;
    void setComponent(int component);

    QString foundation() const;
    void setFoundation(const QString &foundation);

    int generation() const;
    void setGeneration(int generation);

    QHostAddress host() const;
    void setHost(const QHostAddress &host);

    QString id() const;
    void setId(const QString &id);

    int network() const;
    void setNetwork(int network);

    quint16 port() const;
    void setPort(quint16 port);

    int priority() const;
    void setPriority(int priority);

    QString protocol() const;
    void setProtocol(const QString &protocol);

    QXmppJingleCandidate::Type type() const;
    void setType(QXmppJingleCandidate::Type);

    bool isNull() const;

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    static QXmppJingleCandidate::Type typeFromString(const QString &typeStr, bool *ok = nullptr);
    static QString typeToString(QXmppJingleCandidate::Type type);
    /// \endcond

private:
    QSharedDataPointer<QXmppJingleCandidatePrivate> d;
};

class QXMPP_EXPORT QXmppJingleReason
{
public:
    /// This enum is used to describe a reason's type.
    enum Type {
        None,
        AlternativeSession,
        Busy,
        Cancel,
        ConnectivityError,
        Decline,
        Expired,
        FailedApplication,
        FailedTransport,
        GeneralError,
        Gone,
        IncompatibleParameters,
        MediaError,
        SecurityError,
        Success,
        Timeout,
        UnsupportedApplications,
        UnsupportedTransports
    };

    /// Condition of an RTP-specific error
    /// \since QXmpp 1.5
    enum RtpErrorCondition {
        /// There is no error condition.
        NoErrorCondition,
        /// The encryption offer is rejected.
        InvalidCrypto,
        /// Encryption is required but not offered.
        CryptoRequired
    };

    QXmppJingleReason();

    QString text() const;
    void setText(const QString &text);

    Type type() const;
    void setType(Type type);

    RtpErrorCondition rtpErrorCondition() const;
    void setRtpErrorCondition(RtpErrorCondition rtpErrorCondition);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;

    /// \endcond

    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleReason)

private:
    QSharedDataPointer<QXmppJingleIqReasonPrivate> d;
};

///
/// \brief The QXmppJingleIq class represents an IQ used for initiating media
/// sessions as specified by \xep{0166}: Jingle.
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppJingleIq : public QXmppIq
{
public:
    /// This enum is used to describe a Jingle action.
    enum Action {
        ContentAccept,
        ContentAdd,
        ContentModify,
        ContentReject,
        ContentRemove,
        DescriptionInfo,
        SecurityInfo,
        SessionAccept,
        SessionInfo,
        SessionInitiate,
        SessionTerminate,
        TransportAccept,
        TransportInfo,
        TransportReject,
        TransportReplace
    };

    enum Creator {
        /// The initiator generated the content type.
        Initiator,
        /// The responder generated the content type.
        Responder
    };

    struct RtpSessionStateActive
    {
    };

    struct RtpSessionStateHold
    {
    };

    struct RtpSessionStateUnhold
    {
    };

    struct RtpSessionStateMuting
    {
        /// True when temporarily not sending media to the other party but continuing to accept
        /// media from it, false for ending mute state
        bool isMute = true;
        /// Creator of the corresponding session
        Creator creator;
        /// Session to be muted (e.g., only audio or video)
        QString name;
    };

    struct RtpSessionStateRinging
    {
    };

    using RtpSessionState = std::variant<RtpSessionStateActive, RtpSessionStateHold, RtpSessionStateUnhold, RtpSessionStateMuting, RtpSessionStateRinging>;

    /// Alias to QXmppJingleReason for compatibility.
    using Reason = QXmppJingleReason;

    /// \internal
    ///
    /// The QXmppJingleIq::Content class represents the "content" element of a
    /// QXmppJingleIq.
    ///
    class QXMPP_EXPORT Content
    {
    public:
        Content();
        Content(const QXmppJingleIq::Content &other);
        Content(QXmppJingleIq::Content &&);
        ~Content();

        Content &operator=(const Content &other);
        Content &operator=(Content &&);

        QString creator() const;
        void setCreator(const QString &creator);

        QString name() const;
        void setName(const QString &name);

        QString senders() const;
        void setSenders(const QString &senders);

        // XEP-0167: Jingle RTP Sessions
        QXmppJingleDescription description() const;
        void setDescription(const QXmppJingleDescription &description);

#if QXMPP_DEPRECATED_SINCE(1, 6)
        QString descriptionMedia() const;
        void setDescriptionMedia(const QString &media);

        quint32 descriptionSsrc() const;
        void setDescriptionSsrc(quint32 ssrc);

        void addPayloadType(const QXmppJinglePayloadType &payload);
        QList<QXmppJinglePayloadType> payloadTypes() const;
        void setPayloadTypes(const QList<QXmppJinglePayloadType> &payloadTypes);
#endif

        bool isRtpMultiplexingSupported() const;
        void setRtpMultiplexingSupported(bool isRtpMultiplexingSupported);

        std::optional<QXmppJingleRtpEncryption> rtpEncryption() const;
        void setRtpEncryption(const std::optional<QXmppJingleRtpEncryption> &rtpEncryption);

        void addTransportCandidate(const QXmppJingleCandidate &candidate);
        QList<QXmppJingleCandidate> transportCandidates() const;
        void setTransportCandidates(const QList<QXmppJingleCandidate> &candidates);

        QString transportUser() const;
        void setTransportUser(const QString &user);

        QString transportPassword() const;
        void setTransportPassword(const QString &password);

        QVector<QXmppJingleRtpFeedbackProperty> rtpFeedbackProperties() const;
        void setRtpFeedbackProperties(const QVector<QXmppJingleRtpFeedbackProperty> &rtpFeedbackProperties);

        QVector<QXmppJingleRtpFeedbackInterval> rtpFeedbackIntervals() const;
        void setRtpFeedbackIntervals(const QVector<QXmppJingleRtpFeedbackInterval> &rtpFeedbackIntervals);

        QVector<QXmppJingleRtpHeaderExtensionProperty> rtpHeaderExtensionProperties() const;
        void setRtpHeaderExtensionProperties(const QVector<QXmppJingleRtpHeaderExtensionProperty> &rtpHeaderExtensionProperties);

        bool isRtpHeaderExtensionMixingAllowed() const;
        void setRtpHeaderExtensionMixingAllowed(bool isRtpHeaderExtensionMixingAllowed);

        // XEP-0320: Use of DTLS-SRTP in Jingle Sessions
        QByteArray transportFingerprint() const;
        void setTransportFingerprint(const QByteArray &fingerprint);

        QString transportFingerprintHash() const;
        void setTransportFingerprintHash(const QString &hash);

        QString transportFingerprintSetup() const;
        void setTransportFingerprintSetup(const QString &setup);

        /// \cond
        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;

        bool parseSdp(const QString &sdp);
        QString toSdp() const;
        /// \endcond

    private:
        QSharedDataPointer<QXmppJingleIqContentPrivate> d;
    };

    QXmppJingleIq();
    QXmppJingleIq(const QXmppJingleIq &other);
    QXmppJingleIq(QXmppJingleIq &&);
    ~QXmppJingleIq() override;

    QXmppJingleIq &operator=(const QXmppJingleIq &other);
    QXmppJingleIq &operator=(QXmppJingleIq &&);

    Action action() const;
    void setAction(Action action);

    void addContent(const Content &content);
    QList<Content> contents() const;
    void setContents(const QList<Content> &contents);

    QString initiator() const;
    void setInitiator(const QString &initiator);

    QXmppJingleReason &reason();
    const QXmppJingleReason &reason() const;

    QString responder() const;
    void setResponder(const QString &responder);

#if QXMPP_DEPRECATED_SINCE(1, 5)
    QT_DEPRECATED_X("Use QXmpp::rtpSessionState() instead")
    bool ringing() const;
    QT_DEPRECATED_X("Use QXmpp::setRtpSessionState() instead")
    void setRinging(bool ringing);
#endif

    QString sid() const;
    void setSid(const QString &sid);

    QString mujiGroupChatJid() const;
    void setMujiGroupChatJid(const QString &mujiGroupChatJid);

    std::optional<RtpSessionState> rtpSessionState() const;
    void setRtpSessionState(const std::optional<RtpSessionState> &rtpSessionState);

    /// \cond
    static bool isJingleIq(const QDomElement &element);
    /// \endcond

protected:
    /// \cond
    void parseElementFromChild(const QDomElement &element) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QSharedDataPointer<QXmppJingleIqPrivate> d;
};

class QXMPP_EXPORT QXmppJingleMessageInitiationElement
{
public:
    enum class Type {
        None,
        Propose,
        Ringing,
        Proceed,
        Reject,
        Retract,
        Finish
    };

    QXmppJingleMessageInitiationElement();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppJingleMessageInitiationElement)

    Type type() const;
    void setType(Type type);

    QString id() const;
    void setId(const QString &id);

    std::optional<QXmppJingleDescription> description() const;
    void setDescription(std::optional<QXmppJingleDescription> description);

    std::optional<QXmppJingleReason> reason() const;
    void setReason(std::optional<QXmppJingleReason> reason);

    bool containsTieBreak() const;
    void setContainsTieBreak(bool containsTieBreak);

    QString migratedTo() const;
    void setMigratedTo(const QString &migratedTo);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isJingleMessageInitiationElement(const QDomElement &);
    static QString jmiElementTypeToString(Type type);
    static std::optional<Type> stringToJmiElementType(const QString &typeStr);

private:
    QSharedDataPointer<QXmppJingleMessageInitiationElementPrivate> d;
};

class QXMPP_EXPORT QXmppCallInviteElement
{
public:
    enum class Type {
        None,
        Invite,
        Retract,
        Accept,
        Reject,
        Left
    };

    struct Jingle
    {
        QString sid;
        std::optional<QString> jid;

        bool operator==(const Jingle &other) const { return other.sid == sid && other.jid == jid; }

        void parse(const QDomElement &element);
        void toXml(QXmlStreamWriter *writer) const;
    };

    struct External
    {
        QString uri;

        bool operator==(const External &other) const { return other.uri == uri; }

        void toXml(QXmlStreamWriter *writer) const;
    };

    QXmppCallInviteElement();
    QXMPP_PRIVATE_DECLARE_RULE_OF_SIX(QXmppCallInviteElement)

    Type type() const;
    void setType(Type type);

    QString id() const;
    void setId(const QString &id);

    bool audio() const;
    void setAudio(bool audio);

    bool video() const;
    void setVideo(bool video);

    std::optional<Jingle> jingle() const;
    void setJingle(std::optional<Jingle> jingle);

    std::optional<QVector<External>> external() const;
    void setExternal(std::optional<QVector<External>> external);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

    static bool isCallInviteElement(const QDomElement &);

private:
    static QString callInviteElementTypeToString(Type type);
    static std::optional<Type> stringToCallInviteElementType(const QString &typeStr);

    QSharedDataPointer<QXmppCallInviteElementPrivate> d;
};

Q_DECLARE_METATYPE(QXmppJingleReason::RtpErrorCondition)

#endif
