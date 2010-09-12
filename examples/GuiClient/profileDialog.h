#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QMap>

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
    explicit profileDialog(QWidget *parent, const QString& bareJid, QXmppClient& client);
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

private:
    Ui::profileDialog *ui;
    QString m_bareJid;
    QXmppClient& m_xmppClient;  // reference to the active QXmppClient (No ownership)
    QMap<QString, QXmppVersionIq> m_versions;
    QMap<QString, QXmppEntityTimeIq> m_time;
};

#endif // PROFILEDIALOG_H
