#include <QApplication>
#include <QPalette>
#include "classes/client/kk_client.h"
#include "../../libs/src/classes/crdt/kk_crdt.h"
#include "../../libs/src/classes/crdt/char/kk_char.h"
#include <QtCore/QDebug>
#include <QFont>
#include <algorithm>

int main(int argc, char* argv[])
{
    Q_INIT_RESOURCE(client);
    QApplication app(argc, argv);

    // Apply the loaded stylesheet
    QFile styleFile( ":/styles/styles.qss");
    styleFile.open( QFile::ReadOnly );
    app.setStyleSheet(QString( styleFile.readAll()));
    KKClient client(QUrl(QStringLiteral("wss://localhost:3002")));
//    KKClient client(QUrl(QStringLiteral("wss://93.56.104.204:3002")));
    QApplication::setWindowIcon(QIcon(":/klapklap.icns"));
    Q_UNUSED(client)
    return QApplication::exec();
}

void stringDiffInv(std::string str1, std::string str2, unsigned long *lengthX, std::string *stringX );
void stringDiff(std::string str1, std::string str2,unsigned long *lengthX, std::string *stringX);

//int main (int argc, char* argv[]){

//std::string str1 = "iao come va";
//std::string str2 = "ffa";
//std::string str3 = "ciao cff";
//std::string str4 = "ciao come v";
//std::string stringX;
//unsigned long lengthX;

//stringDiffInv(str1,str2,&lengthX,&stringX);
//std::cout<< stringX << " " << lengthX << "\n";

//stringDiff(str3,str4,&lengthX,&stringX);
//std::cout<< stringX << " " << lengthX << "\n";

//return 0;
//}

void stringDiffInv(std::string str1, std::string str2,unsigned long *lengthX, std::string *stringX){
    std::reverse(str1.begin(),str1.end());
    std::reverse(str2.begin(),str2.end());
    std::string tmp;
    unsigned long i;

   for(i=0; i < std::max(str1.length(),str2.length()); i++){
       if(str1[i]!=str2[i]){
           *lengthX= str1.length()-i;
           tmp = str2.substr(i,string::npos);
           std::reverse(tmp.begin(),tmp.end());
           *stringX = tmp;
           return;
       }
  }
}

void stringDiff(std::string str1, std::string str2,unsigned long *lengthX, std::string *stringX){
    unsigned long i;

   for(i=0; i < std::max(str1.length(),str2.length()); i++){
        if(str1[i]!=str2[i]){
            *lengthX=i;
            *stringX = str1.substr(i,string::npos);
            return;
        }
    }
}
