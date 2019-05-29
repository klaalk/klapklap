//
// Created by Michele Luigi Greco on 2019-05-29.
//

#ifndef SERVER_QSMTP_SERVICE_H
#define SERVER_QSMTP_SERVICE_H
#include "./classes/QSMTP_service/src/SmtpMime"


class QSMTP_service {
private:
    

public:

    bool QSMTP_send_message(QString message, QString dest_name ,QString dest_mail, QString subject);

    QString
    QSMTP_message_builder(QString header, QString username, QString message, QString button_text,
                         QString button_action);
};


#endif //SERVER_QSMTP_SERVICE_H
