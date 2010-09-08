#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H
#include "ui_statusWidget.h"
#include "QXmppPresence.h"

class statusWidget : public QWidget, public Ui::statusWidgetClass
{
    Q_OBJECT

public:
    statusWidget(QWidget* parent = 0);
    void setDisplayName(const QString& name);
    void setStatusText(const QString& statusText);
    void setPresenceAndStatusType(QXmppPresence::Type presenceType,
                                  QXmppPresence::Status::Type statusType);
    void setAvatar(const QImage&);

private slots:
    void presenceMenuTriggered();
    void avatarSelection();

signals:
    void statusTextChanged(const QString&);
    void presenceTypeChanged(QXmppPresence::Type);
    void presenceStatusTypeChanged(QXmppPresence::Status::Type);
    void avatarChanged(const QImage&);
};

#endif // STATUSWIDGET_H
