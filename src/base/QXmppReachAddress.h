#ifndef QXMPPREACHADDRESS_H
#define QXMPPREACHADDRESS_H


#include "QXmppStanza.h"
#include "QXmppElement.h"

#include <QList>

/// \brief The QXmppReachAddress class represents a reach element in a event item
/// as defined by XEP-0152: Reachability Addresses.


class QXMPP_EXPORT QXmppAddress
{
public:
    QXmppAddress();
    QXmppAddress(const QString& addr, const QString& desc, const QString& lang);
    void setAddress(const QString& addr);
    void setDescription(const QString& desc);
    void setLanguage(const QString& lang);
    const QString& getAddress() const;
    const QString& getDescription() const;
    const QString& getLanguage() const;

private:
    QString m_address;
    QString m_description;
    QString m_language;
};

class QXMPP_EXPORT QXmppReachAddress
{
public:
    QXmppReachAddress();

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond
    ///


    QXmppElement toQXmppElement() const;

    bool isNull() const;
    const QList <QXmppAddress> getAddresses() const;
    void addAddress(const QXmppAddress& addr);

private:
    QList <QXmppAddress> m_addressList;
};




#endif // QXMPPREACHADDRESS_H
