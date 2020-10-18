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

// Thus, we should use multiple inheritance: QObject inserted here
class KKTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    KKTask(std::function<void ()> f_taks);
signals:

protected:
    void run();
private:
    std::function<void()> function;
};

#endif // KKTASK_H
