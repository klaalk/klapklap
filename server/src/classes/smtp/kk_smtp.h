#ifndef SERVER_KK_SMTP_H
#define SERVER_KK_SMTP_H

#include "./classes/smtp/mime/SmtpMime.h"

#include "../../../../libs/src/classes/crypt/kk_crypt.h"
#include "../../../../libs/src/constants/kk_constants.h"

#include <classes/user/kk_user.h>

class KKSmtp {
private:
    bool sendMessage(QString message, QString dest_name, QString dest_mail, QString subject);
    QString messageBuilder(QString header, QString username, QString message, QString button_text, QString button_action);

public:

    int sendSignupEmail(QString username, QString email, QString name, QString surname);
    int sendAddUserFileEmail(KKUserPtr user, QString filename);
    int sendShareUserFileEmail(QString filename, KKUserPtr fromUser, KKUserPtr toUser);

};

typedef QSharedPointer<KKSmtp> KKSmtpPtr;
#endif //SERVER_KK_SMTP_H
