#ifndef QXMPPSIMPLEARCHIVEPREFERENCEIQ_H
#define QXMPPSIMPLEARCHIVEPREFERENCEIQ_H

#include "QXmppIq.h"
#include "QXmppResultSet.h"

#include <QDateTime>

class QXmlStreamWriter;
class QDomElement;

/// \brief Represents an archive preference query as defined by
/// XEP-0313: Message Archive Management
///
/// It is used to get & set preferences
///
/// \ingroup Stanzas

class QXMPP_EXPORT QXmppSimpleArchivePreferenceIq : public QXmppIq
{
public:
    enum QXmppArchivePreference {
        Always = 0,
        Never,
        Roster
    };

    QXmppSimpleArchivePreferenceIq(const QXmppArchivePreference& def = Always);

    static bool isSimpleArchivePreferenceIq(const QDomElement &element);

    QXmppArchivePreference archiveDefault() const;
    void setArchiveDefault(const QXmppArchivePreference& def);

    QList<QString> alwaysArchive() const;
    void addAlwaysArchive(const QString& jid);

    QList<QString> neverArchive() const;
    void addNeverArchive(const QString& jid);

protected:
    void parseElementFromChild(const QDomElement &element);
    void toXmlElementFromChild(QXmlStreamWriter *writer) const;
private:
    QXmppArchivePreference m_default;
    QList<QString> m_always;
    QList<QString> m_never;
};

#endif // QXMPPSIMPLEARCHIVEPREFERENCEIQ_H
