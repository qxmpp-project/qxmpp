#ifndef QXMPP_GAMING_IQ_H
#define QXMPP_GAMING_IQ_H

#include "QXmppIq.h"
#include "QXmppResultSet.h"

#include "QXmppElement.h"

/// \brief The QXmppGaming class represents an IQ for conveying a
/// user gaming as defined by:
/// \li XEP-0196: User Gaming
///
/// \ingroup Stanzas
///
class QXMPP_EXPORT QXmppGaming
{
public:
    QXmppGaming();

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond
    ///

    QXmppElement toQXmppElement() const;

    QString characterName() const;
    void setCharacterName(const QString &characterName);

    QString characterProfile() const;
    void setCharacterProfile(const QString &characterProfile);

    QString name() const;
    void setName(const QString &name);

    QString level() const;
    void setLevel(const QString &level);

    QString serverAddress() const;
    void setServerAddress(const QString &serverAddress);

    QString serverName() const;
    void setServerName(const QString &serverName);

    QString uri() const;
    void setUri(const QString &uri);

private:
    void writeToElement(QXmppElement& element, const QString& name, const QString& value) const;

    QString m_character_name;
    QString m_character_profile;
    QString m_name;
    QString m_level;
    QString m_server_address;
    QString m_server_name;
    QString m_uri;
};


#endif // QXMPP_GAMING_IQ_H
