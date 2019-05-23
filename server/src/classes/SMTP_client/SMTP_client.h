//
// Created by jsnow on 16/05/19.
//

#ifndef SERVER_SMTP_CLIENT_H
#define SERVER_SMTP_CLIENT_H
#include "../../../../libs/src/classes_include.h"



using namespace std;


class SMTP_client {

private:
    string host = "smtp.gmail.com";
    int port = 465;
    std::string sUserName = "klapklap.assistence@gmail.com";
    std::string sPassword = "Progetto2019";
    SharedPtr<InvalidCertificateHandler> pCert = new AcceptCertificateHandler(false);
    Context::Ptr pContext = new Poco::Net::Context(Context::CLIENT_USE, "", "", "", Context::VERIFY_NONE, 9, false,
                                                   "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");


public:
    int SMPT_sendmail(std::string message, std::string dest, std::string subject);

    std::string
    SMTP_message_builder(std::string header, std::string username, std::string message, std::string button_text,
                         std::string button_action);
};


#endif //SERVER_SMTP_CLIENT_H
