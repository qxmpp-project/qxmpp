/*
 * Copyright (C) 2020 The QXmpp developers
 *
 * Author:
 *  Niels Ole Salscheider
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
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

#ifndef QXMPPCALLSTREAM_H
#define QXMPPCALLSTREAM_H

#include <QXmppGlobal.h>

#include <gst/gst.h>

#include <functional>

#include <QObject>

class QXmppCallStreamPrivate;
class QXmppIceConnection;
class QXmppCall;
class QXmppCallPrivate;

/// \brief The QXmppCallStream class represents an RTP stream in a VoIP call.
///
/// \note THIS API IS NOT FINALIZED YET
///
/// \since QXmpp 1.3

class QXMPP_EXPORT QXmppCallStream : public QObject
{
    Q_OBJECT

public:
    QString creator() const;
    QString media() const;
    QString name() const;
    int id() const;
    void setReceivePadCallback(std::function<void(GstPad *)> cb);
    void setSendPadCallback(std::function<void(GstPad *)> cb);

private:
    QXmppCallStream(GstElement *pipeline, GstElement *rtpbin,
                    QString media, QString creator, QString name, int id);

    QXmppCallStreamPrivate *d;

    friend class QXmppCall;
    friend class QXmppCallPrivate;
};

#endif
