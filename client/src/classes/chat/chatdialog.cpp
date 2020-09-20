/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <utility> 
#include "chatdialog.h"

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    lineEdit->setFocusPolicy(Qt::StrongFocus);
    textEdit->setFocusPolicy(Qt::NoFocus);
    textEdit->setReadOnly(true);
    listWidget->setFocusPolicy(Qt::NoFocus);
    connect(listWidget, &QListWidget::itemClicked, this, &ChatDialog::onItemClicked);
    connect(lineEdit, &QLineEdit::returnPressed, this, &ChatDialog::returnPressed);
    colorOnline = new QColor(25, 118, 210);
    colorOffline = new QColor(33, 33, 33);
}

void ChatDialog::resetState(){
    lineEdit->clear();
    textEdit->clear();
    listWidget->clear();
    myNickName = "";
}

void ChatDialog::setNickName(QString nick) {
    myNickName = std::move(nick);
    tableFormat.setBorder(0);
}

void ChatDialog::appendMessage(const QString &from, const QString &message)
{
    if (from.isEmpty() || message.isEmpty())
        return;

    QTextCursor cursor(textEdit->textCursor());
    cursor.movePosition(QTextCursor::End);
    QTextTable *table = cursor.insertTable(1, 2, tableFormat);
    table->cellAt(0, 0).firstCursorPosition().insertText(from + " > ");
    table->cellAt(0, 1).firstCursorPosition().insertText(message);
    QScrollBar *bar = textEdit->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void ChatDialog::returnPressed()
{
    QString text = lineEdit->text();
    if (text.isEmpty())
        return;

    if (text.startsWith(QChar('/'))) {
        QColor color = textEdit->textColor();
        textEdit->setTextColor(Qt::red);
        textEdit->append(tr("! Unknown command: %1")
                         .arg(text.left(text.indexOf(' '))));
        textEdit->setTextColor(color);
    } else {
        emit sendMessageEvent(myNickName, text);
    }
    lineEdit->clear();
}

void ChatDialog::addParticipant(const QString &username, const QString &nick, const QString &avatar)
{
    if (nick.isEmpty())
        return;

    setParticipantState(username, nick, avatar, PARTICIPANT_ONLINE);

    QColor color = textEdit->textColor();
    textEdit->setTextColor(Qt::gray);
    textEdit->append(tr("* %1 has joined").arg(nick));
    textEdit->setTextColor(color);
}

void ChatDialog::removeParticipant(const QString &username, const QString &nick)
{

    setParticipantState(username, nick, QString(), PARTICIPANT_OFFLINE);

    QColor color = textEdit->textColor();
    textEdit->setTextColor(Qt::gray);
    textEdit->append(tr("* %1 has left").arg(nick));
    textEdit->setTextColor(color);

    emit siteIdClicked(username, true);
}

void ChatDialog::setParticipants(const QStringList participants)
{
    for(QString participant : participants) {

        QStringList params = participant.split(":");
        QString username = params.at(0);
        QString nick = params.at(1);
        QString avatar = params.at(2);
        QString state = params.at(3);
        this->participants.insert(username, state);

        QListWidgetItem* item = new QListWidgetItem();
        item->setWhatsThis(username);
        item->setText(nick);
        item->setIcon(QIcon(":/images/avatars/"+avatar));
        if (state == PARTICIPANT_ONLINE)
           item->setTextColor(*colorOnline);
        else
           item->setTextColor(*colorOffline);

        listWidget->addItem(item);
    }
}

void ChatDialog::showInformation()
{
    if (listWidget->count() == 1) {
        QMessageBox::information(this, tr("Chat"),
                                 tr("Launch several instances of this "
                                    "program on your local network and "
                                    "start chatting!"));
    }
}

void ChatDialog::onItemClicked(QListWidgetItem *item) {
    if(!item) {
        return;
    }
    QString siteId = item->whatsThis();
    emit siteIdClicked(siteId, false);
}

void ChatDialog::setParticipantState(const QString &username, const QString &nick, const QString &avatar, const QString &state)
{
    if (!nick.isEmpty()) {
        QListWidgetItem* item = findParticipantItem(nick);

        if (item == nullptr) {
            item = new QListWidgetItem();
            item->setWhatsThis(username);
            item->setText(nick);
            item->setIcon(QIcon(":/images/avatars/"+avatar));
            listWidget->addItem(item);
        }

        if (state == PARTICIPANT_ONLINE)
           item->setTextColor(*colorOnline);
        else
           item->setTextColor(*colorOffline);

        if (!nick.isEmpty())
            item->setText(nick);

        if (!avatar.isEmpty())
            item->setIcon(QIcon(":/images/avatars/"+avatar));
    }
    participants_label->setText(QString("Participants (%1)").arg(participants.values().length()));
}

QListWidgetItem *ChatDialog::findParticipantItem(const QString &nick)
{
    QList<QListWidgetItem *> items = listWidget->findItems(nick, Qt::MatchExactly);
    if (items.isEmpty())
        return nullptr;

    return items.at(0);
}

