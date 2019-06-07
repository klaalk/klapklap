//
// Created by Michele Luigi Greco on 2019-05-29.
//

#ifndef SERVER_KK_SMTP_H
#define SERVER_KK_SMTP_H

#include "./classes/smtp/mime/SmtpMime.h"


class kk_smtp {
private:


public:

    bool QSMTP_send_message(QString message, QString dest_name, QString dest_mail, QString subject);

    QString
    QSMTP_message_builder(QString header, QString username, QString message, QString button_text,
                          QString button_action);
};


#endif //SERVER_KK_SMTP_H
