// Create by Klaus Cuko

#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <utility>

#include <QtWidgets>

#include "../../../../libs/src/constants/kk_constants.h"

#include "ui_chatdialog.h"

class KKChat : public QDialog, private Ui::ChatDialog
{
    Q_OBJECT

public:
    KKChat(QWidget *parent = nullptr);
    void resetState();
    void setNickName(QString nick);
    void appendMessage(const QString &from, const QString &message);
    void addParticipant(const QString &username, const QString &nick, const QString &avatar);
    void removeParticipant(const QString &username, const QString &nick, const QString &avatar);
    void setParticipants(const QStringList participants);
    void setParticipantChatBackgroundColor(QBrush color, QString siteId);
    QPixmap createAvatarIcon(const QString &avatar, const QString &state);

signals:
    void sendMessageEvent(QString username, QString message);
    void siteIdClicked(QString nickname);

private slots:
    void returnPressed();
    void showInformation();
    void onItemClicked(QListWidgetItem *item);
    void setParticipantState(const QString &username, const QString &nick, const QString &avatar, const QString &state);
    QListWidgetItem* findParticipantItem(const QString &nick, const QString &siteId);

private:
    QString myNickName;
    QTextTableFormat tableFormat;
    QMap<QString, QString> participants;
    QColor* colorOnline;
    QColor* colorOffline;
    QMap<QString,QString> siteIdNickName;
};

#endif
