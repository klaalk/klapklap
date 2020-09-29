#include "kk_chat.h"

KKChat::KKChat(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    lineEdit->setFocusPolicy(Qt::StrongFocus);
    textEdit->setFocusPolicy(Qt::NoFocus);
    textEdit->setReadOnly(true);
    listWidget->setFocusPolicy(Qt::NoFocus);
    connect(listWidget, &QListWidget::itemClicked, this, &KKChat::onItemClicked);
    connect(lineEdit, &QLineEdit::returnPressed, this, &KKChat::returnPressed);
    colorOnline = new QColor(25, 118, 210);
    colorOffline = new QColor(33, 33, 33);
}

void KKChat::resetState(){
    lineEdit->clear();
    textEdit->clear();
    listWidget->clear();
    myNickName = "";
}

void KKChat::setNickName(QString nick) {
    myNickName = std::move(nick);
    tableFormat.setBorder(0);
}

void KKChat::appendMessage(const QString &from, const QString &message)
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

void KKChat::returnPressed()
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

void KKChat::addParticipant(const QString &username, const QString &nick, const QString &avatar)
{
    if (nick.isEmpty())
        return;

    setParticipantState(username, nick, avatar, PARTICIPANT_ONLINE);

    QColor color = textEdit->textColor();
    textEdit->setTextColor(Qt::gray);
    textEdit->append(tr("* %1 has joined").arg(nick));
    textEdit->setTextColor(color);
}

void KKChat::removeParticipant(const QString &username, const QString &nick)
{

    setParticipantState(username, nick, QString(), PARTICIPANT_OFFLINE);

    QColor color = textEdit->textColor();
    textEdit->setTextColor(Qt::gray);
    textEdit->append(tr("* %1 has left").arg(nick));
    textEdit->setTextColor(color);
}

void KKChat::setParticipants(const QStringList participants)
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
        siteIdNickName.insert(username,nick);
        item->setText(nick);

        const QPixmap orig = QPixmap(":/images/avatars/"+avatar);

        int size = qMax(orig.width(), orig.height());

        QPixmap rounded = QPixmap(size, size);
        rounded.fill(Qt::transparent);

        QPainterPath path;
        path.addEllipse(rounded.rect());

        QPainter painter(&rounded);
        painter.setClipPath(path);

        painter.fillRect(rounded.rect(), Qt::black);

        int x = qAbs(orig.width() - size) / 2;
        int y = qAbs(orig.height() - size) / 2;
        painter.drawPixmap(x, y, orig.width(), orig.height(), orig);


        item->setIcon(QIcon(rounded));
        if (state == PARTICIPANT_ONLINE)
           item->setTextColor(*colorOnline);
        else
           item->setTextColor(*colorOffline);

        QFont tmp ;
        tmp.setPointSizeF(12);
        item->setFont(tmp);

        listWidget->setIconSize(QSize(25,25));
        listWidget->addItem(item);
    }
}

void KKChat::showInformation()
{
    if (listWidget->count() == 1) {
        QMessageBox::information(this, tr("Chat"),
                                 tr("Launch several instances of this "
                                    "program on your local network and "
                                    "start chatting!"));
    }
}

void KKChat::onItemClicked(QListWidgetItem *item) {
    if(!item) {
        return;
    }
    QString siteId = item->whatsThis();
    emit siteIdClicked(siteId);
}

void KKChat::setParticipantChatBackgroundColor(QBrush color, QString siteId){
    QListWidgetItem* item = findParticipantItem(siteIdNickName.value(siteId),siteId);
    QColor tmp = color.color();
    tmp.setAlpha(color.color().alpha()-40);
    item->setBackgroundColor(tmp);
    item->setSelected(false);
}


void KKChat::setParticipantState(const QString &username, const QString &nick, const QString &avatar, const QString &state)
{
    if (!nick.isEmpty()) {
        QListWidgetItem* item = findParticipantItem(nick,username);

        const QPixmap orig = QPixmap(":/images/avatars/"+avatar);

        int size = qMax(orig.width(), orig.height());

        QPixmap rounded = QPixmap(size, size);
        rounded.fill(Qt::transparent);

        QPainterPath path;
        path.addEllipse(rounded.rect());

        QPainter painter(&rounded);
        painter.setClipPath(path);

        painter.fillRect(rounded.rect(), Qt::black);

        int x = qAbs(orig.width() - size) / 2;
        int y = qAbs(orig.height() - size) / 2;
        painter.drawPixmap(x, y, orig.width(), orig.height(), orig);

        if (item == nullptr) {
            item = new QListWidgetItem();
            item->setWhatsThis(username);
            item->setText(nick);
            siteIdNickName.insert(username,nick);
            item->setIcon(QIcon(rounded));
            listWidget->addItem(item);
        }

        if (state == PARTICIPANT_ONLINE)
           item->setTextColor(*colorOnline);
        else
           item->setTextColor(*colorOffline);

        if (!nick.isEmpty())
            item->setText(nick);

        if (!avatar.isEmpty())
            item->setIcon(QIcon(rounded));
    }
    participants_label->setText(QString("Participants (%1)").arg(participants.values().length()));
}

QListWidgetItem *KKChat::findParticipantItem(const QString &nick, const QString &siteId){
    QList<QListWidgetItem *> items = listWidget->findItems(nick, Qt::MatchExactly);
    if (items.isEmpty()){
        return nullptr;
    }
    for(QListWidgetItem * item : items){
        if(item->whatsThis() == siteId)
            return item;
    }
    return nullptr;
}

