//
// Created by Klaus on 06/05/2019.
//


#ifndef KKTASK_H
#define KKTASK_H

#include <QRunnable>
#include <QObject>
#include <QRunnable>
#include <functional>
#include <utility>
// Q_OBJECT missing in the original file generated by class wizard.
// because we set this class with base class QRunnable
// with no inheritance in the class wizard
// We do not have this. So, we cannot use signal/slot
// But we need them.
// Thus, we should use multiple inheritance: QObject inserted here

class kk_task : public QObject, public QRunnable
{
    Q_OBJECT
public:
    kk_task(std::function<bool()> f);

signals:
    // notify to the main thread when we're done
    void Result(bool result);

protected:
    void run();
private:
    std::function<bool()> taskFunc;
};

#endif // KKTASK_H