#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QMap>
#include "capabilitiesCollection.h"

namespace Ui {
    class profileDialog;
}

class QXmppClient;
class QXmppVersionIq;
class QXmppEntityTimeIq;

class profileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit profileDialog(QWidget *parent, const QString& bareJid, QXmppClient& client, capabilitiesCollection& caps);
    ~profileDialog();

    void setClientRef(QXmppClient& m_xmppClient);
    void setAvatar(const QImage&);
    void setBareJid(const QString&);
    void setFullName(const QString&);
    void setStatusText(const QString&);

private slots:
    void versionReceived(const QXmppVersionIq&);
    void timeReceived(const QXmppEntityTimeIq&);

private:
    void updateText();
    QString getCapability(const QString& resource);

private:
    Ui::profileDialog *ui;
    QString m_bareJid;
    QXmppClient& m_xmppClient;  // reference to the active QXmppClient (No ownership)
    capabilitiesCollection& m_caps;  // reference to the active QXmppClient (No ownership)
    QMap<QString, QXmppVersionIq> m_versions;
    QMap<QString, QXmppEntityTimeIq> m_time;
};

#endif // PROFILEDIALOG_H
