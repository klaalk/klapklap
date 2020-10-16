#include "kk_logger.h"

#include <QDateTime>
#include <QSharedPointer>
#include <QTextStream>
#include <QDebug>
#include <QStandardPaths>

KKLogger::KKLogger(){}
QSharedPointer<QFile> KKLogger::logFile = nullptr;

void KKLogger::log(QString message, QString identifier)
{
    QString path = LOG_ROOT + QDateTime::currentDateTime().toString("dd.MM.yyyy") + "_log.txt";

    if (!QFile().exists(path)) {
        if (!logFile.isNull()) {
            logFile->deleteLater();
        }
        logFile = nullptr;
    }

    if (logFile == nullptr)
        logFile = QSharedPointer<QFile>(new QFile (path));

    bool result = logFile->open(QIODevice::Append | QIODevice::Text);
    if (result) {
        QString printMessage = message;
        if (printMessage.size() > 60) {
            printMessage.truncate(50);
            printMessage.append("[...]");
            printMessage.append(message.mid(message.length() - 10));
        }
        printMessage.insert(0, QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss - [") + identifier + "] - ");
        QTextStream stream(logFile.get());
        stream << printMessage << endl;
        logFile->close();
        qDebug() << "[log] " + printMessage;
    }
}
