//
// Created by michele on 09/06/2019.
//

#include "kk_filesys.h"

bool kk_filesys::kk_CreateFile(QString username, QString filename){

    SimpleCrypt crypt(Q_UINT64_C(0x0c2ad4a4acb9f023));
    QString tmp;

    // Serve ad evitare che il carattere "/" dia problemi nei path
    do{
        tmp= crypt.encryptToString(username);
    }
    while(crypt.containLetter('/',tmp));

    // Serve per garantire l'univocità del path.
    QString jump;
    jump=crypt.random_psw(jump);

    QString _filename = jump+"_"+tmp+"_"+filename;
    QString command = "touch ./"+_filename;
    system(qPrintable(command));
    system(qPrintable("ls -la | grep "+ _filename));

    return true;
//    return db->db_insert_file(username,_filename,"./"+_filename) == 0 ? true:false;
}
