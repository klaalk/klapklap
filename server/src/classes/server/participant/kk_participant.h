//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_PARTECIPANT_H
#define KK_PARTECIPANT_H

#include <iostream>
#include <deque>
#include <QString>
#include <QObject>
#include "../../../../../libs/src/classes/payload/kk_payload.h"

class kk_participant : public QObject {
    Q_OBJECT
protected:
    QString name;
public:
    kk_participant(QObject *parent = 0);
    ~kk_participant() {}
    void deliver(const kk_payload &msg);
};

typedef std::deque<kk_payload> kk_payload_queue;
#endif //KK_PARTECIPANT_H
