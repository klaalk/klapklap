//
// Created by Michele Luigi Greco on 2019-05-29.
//

#ifndef SERVER_KK_SMTP_H
#define SERVER_KK_SMTP_H

#include "./classes/smtp/mime/SmtpMime.h"


class KKSmtp {
private:
public:
    bool sendMessage(QString message, QString dest_name, QString dest_mail, QString subject);
    QString messageBuilder(QString header, QString username, QString message, QString button_text, QString button_action);
};

#endif //SERVER_KK_SMTP_H
