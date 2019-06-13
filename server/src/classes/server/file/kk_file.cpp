//
// Created by Klaus on 06/05/2019.
//

#include "kk_file.h"

kk_file::kk_file(){
    recent_msgs_ = QSharedPointer<QVector<kk_payload_ptr>>(new QVector<kk_payload_ptr>());
}

kk_file::~kk_file() {
    std::for_each(recent_msgs_->begin(), recent_msgs_->end(), [](kk_payload_ptr d){
        delete d.get();
    });

    delete recent_msgs_.get();
}
void kk_file::join(QSharedPointer<kk_participant> participant) {
    participants_.insert(participant);
}

void kk_file::leave(QSharedPointer<kk_participant> participant) {
    participants_.erase(participant);
}

void kk_file::deliver(QString type, QString result, QString message, QString myNick) {
    kk_payload_ptr data = QSharedPointer<kk_payload>(new kk_payload(type,result, message));
    recent_msgs_->push_back(data);

    while (recent_msgs_->size() > max_recent_msgs)
        recent_msgs_->pop_front();

    std::for_each(participants_.begin(), participants_.end(),[&](QSharedPointer<kk_participant> p){
        if(p->nick_ != myNick && myNick != "All") {
            p->deliver(data);
        }
    });
}

queue_payload_ptr kk_file::getRecentMessages() {
    return recent_msgs_;
}
