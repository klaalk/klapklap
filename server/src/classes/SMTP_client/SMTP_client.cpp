//
// Created by jsnow on 16/05/19.
//

#include "SMTP_client.h"


int SMTP_client::SMPT_sendmail(std::string to_user, std::string subject, std::string message){
    MailMessage _message;

    try{
        SSLManager::instance().initializeClient(0, pCert, pContext);
        SecureStreamSocket pSSLSocket(pContext);
        pSSLSocket.connect(SocketAddress(host, port));
        SecureSMTPClientSession secure(pSSLSocket);

        secure.login();
        bool tlsStarted = secure.startTLS(pContext);
        secure.login(SMTPClientSession::AUTH_LOGIN, sUserName, sPassword);

        _message.setSender("KlapKlap Soft <klapklap.assistence@gmail.com>");
        _message.addRecipient(MailRecipient(MailRecipient::PRIMARY_RECIPIENT, to_user));
        _message.setSubject(subject);
        _message.setContentType("text/plain; charset=UTF-8");
        _message.setContent(message, MailMessage::ENCODING_8BIT);

        secure.sendMessage(_message);
        secure.close();
    } catch (Poco::Net::SMTPException &e) {
        cout << e.code() << endl;
        cout << e.message() << endl;
        cout << e.what() << endl;
        cout << e.displayText().c_str() << endl;
    }
    catch (Poco::Net::NetException &e) {
        cout << e.code() << endl;
        cout << e.message() << endl;
        cout << e.what() << endl;
        cout << e.displayText().c_str() << endl;
    }
}
