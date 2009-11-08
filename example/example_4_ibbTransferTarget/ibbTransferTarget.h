/*
 * Copyright (C) 2008-2009 QXmpp Developers
 *
 * Author:
 *	Ian Reinhart Geiser
 *
 * Source:
 *	http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */


#ifndef IBBTRANSFERTARGET_H
#define IBBTRANSFERTARGET_H

#include "QXmppClient.h"

class QBuffer;
class IbbTransferTarget : public QXmppClient
{
    Q_OBJECT

public:
    IbbTransferTarget(QObject *parent = 0);
    ~IbbTransferTarget();

public slots:
    void openReceived( const QString&, const QString& );
    void closeReceived( const QString&, const QString& );
private:
    QBuffer *m_buffer;
};

#endif // IBBTRANSFERTARGET_H
