#ifndef QXMPPINFORMATIONREQUESTRESULT_H
#define QXMPPINFORMATIONREQUESTRESULT_H

#include "QXmppIq.h"

class QXmppInformationRequestResult : public QXmppIq
{
public:
    QXmppInformationRequestResult();
    virtual void toXmlElementFromChild(QXmlStreamWriter *writer) const;
};

#endif // QXMPPINFORMATIONREQUESTRESULT_H
