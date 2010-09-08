#include "customListView.h"
#include "rosterItem.h"
#include <QApplication>
#include <QMenu>
#include <QKeyEvent>

customListView::customListView(QWidget* parent):QListView(parent), m_chat("Chat", this), m_profile("View Profile", this)
{
    bool check = connect(this, SIGNAL(pressed(const QModelIndex&)), this,
                         SLOT(mousePressed(const QModelIndex&)));
    Q_ASSERT(check);
    check = connect(this, SIGNAL(doubleClicked(const QModelIndex&)), this,
                         SLOT(doubleClicked(const QModelIndex&)));
    Q_ASSERT(check);
    check = connect(this, SIGNAL(clicked(const QModelIndex&)), this,
                         SLOT(clicked(const QModelIndex&)));
    Q_ASSERT(check);
    check = connect(&m_chat, SIGNAL(triggered()), this,
                         SLOT(showChatDialog_helper()));
    Q_ASSERT(check);
}

bool customListView::event(QEvent* e)
{
    return QListView::event(e);
}

void customListView::mousePressed(const QModelIndex& index)
{
    if(QApplication::mouseButtons() == Qt::RightButton)
    {
        QString bareJid = index.data().toString();
        QMenu menu(this);
        menu.addAction(&m_chat);
        menu.setDefaultAction(&m_chat);
        menu.addAction(&m_profile);
        menu.exec(QCursor::pos());
    }
}

void customListView::doubleClicked(const QModelIndex& index)
{
    m_chat.trigger();
}

void customListView::clicked(const QModelIndex& index)
{
}

void customListView::showChatDialog_helper()
{
    QString bareJid;
    if(selectedIndexes().size() > 0)
    {
        bareJid = selectedIndexes().at(0).data(rosterItem::BareJid).toString();

        if(!bareJid.isEmpty())
            emit showChatDialog(bareJid);
    }
}

void customListView::keyPressEvent(QKeyEvent* event1)
{
    if(event1->key() == Qt::Key_Return)
    {
        showChatDialog_helper();
    }
    QListView::keyPressEvent(event1);
}
