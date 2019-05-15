//
// Created by Klaus on 06/05/2019.
//

#ifndef SERVER_CRDT_FILE_H
#define SERVER_CRDT_FILE_H

#include <set>
#include <algorithm>
#include <boost/bind.hpp>
#include "../../../../../libs/src/classes/chat/message/chat_message.h"
#include "../partecipant/crdt_partecipant.h"


class crdt_file {
public:
    void join(crdt_participant_ptr participant);

    void leave(crdt_participant_ptr participant);

    void deliver(const chat_message &msg);

private:
    std::set<crdt_participant_ptr> participants_;
    enum {
        max_recent_msgs = 100
    };
    crdt_message_queue recent_msgs_;
};

#endif //SERVER_CRDT_FILE_H
