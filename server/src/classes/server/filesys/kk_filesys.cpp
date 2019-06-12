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

    return db->db_insert_file(username,_filename,"./"+_filename) == 0 ? true:false;
}

bool kk_filesys::kk_OpenFile(QString username, QString filename){

    SimpleCrypt crypt(Q_UINT64_C(0x0c2ad4a4acb9f023));
    // 0=random_jump, 1=crypted username 2=effective filename
    QStringList body = filename.split("_");
    QString _username=crypt.decryptToString(body[1]);

    if (db->db_exist_user(_username)==false)
        return false;
    //Gia presente nella tabella, avendo
    if(username==_username)
        return true;

    //    sto aprendo un file al quale sono invitato, devo tenerne traccia sul db
    return db->db_insert_file(username,filename,"./"+filename) == 0 ? true:false;


}

bool kk_filesys::kk_SendFile(QString filename){

    QFile file(filename);
    file.open(QFile::ReadOnly);

    //@klaus
    //va inviato il file tramite la sock


    return true;
}

//@klaus in pratica, come dicevamo, creiamo un file con "jump_crypt(username)_filename.txt".
// dove jump è una sringa causale da (26+10)^20 combinazioni che ci garantisce univocità per
// il filename completo. Viene chiamata la open file che: se lo user non esiste da false,
// se user è proprietario del file da true, non c'e bisogno di fare altre operazioni,
// se l'user non è proprietario vuol dire che è stato invitato, quindi va aggiunto,
// il valore ritornato dipende da se riesce o meno la query nel db.
// SendFile prene il file e lo invia sulla sock che usiamo per comunicare
// con header "file_response" e payload "<binary file content>"














