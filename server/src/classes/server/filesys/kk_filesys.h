//
// Created by michele on 09/06/2019.
//

#ifndef KK_FILESYS_H
#define KK_FILESYS_H

#include <set>
#include <iostream>
#include <algorithm>
#include <functional>
#include <memory>
#include <QSharedPointer>
#include <QWebSocket>
#include <QFile>
#define FILE_PATH "./filefolder/"


class kk_filesys {
public:
    kk_filesys();
    ~kk_filesys();

//    void filesysOpenFile(QString username, QString filename, QString path){
//        QString command="touch "+ path ;
//        QFile file(filename);
//        QStringList list;

//        if(files_tree.find(file)==files_tree.end()){
//            system(qPrintable(command));
//            list.push_back(username);
//            files_tree.insert(file,list);


//        }else {
//            files_tree.find(file)->push_back(username);
//        }

//    }



private:
    QMap <QFile,QStringList>files_tree; //chiave: puntatore al file, value: lista degli utenti conessi a quel file

};
typedef QSharedPointer<kk_filesys> kk_filesys_ptr;
#endif //KK_FILESYS_H
