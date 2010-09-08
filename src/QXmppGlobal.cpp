#include "QXmppGlobal.h"

#define QXMPPSTRINGIZE(n)    _QXMPPSTRINGIZE(n)
#define _QXMPPSTRINGIZE(n)    #n

QString QXmppVersion()
{
    return QString(QXMPPSTRINGIZE(QXMPP_VERSION));
}

