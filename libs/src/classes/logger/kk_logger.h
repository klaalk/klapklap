#ifndef KKLOGGER_H
#define KKLOGGER_H

#include <QFile>

#include "../../constants/kk_constants.h"

class KKLogger
{
public:
    KKLogger();
    static QSharedPointer<QFile> logFile;
    static void log(QString message, QString identifier);
};

#endif // KKLOGGER_H
