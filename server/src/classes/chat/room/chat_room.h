//
// Created by Augens on 06/05/2019.
//

#ifndef SERVER_CHAT_ROOM_H
#define SERVER_CHAT_ROOM_H

#include <set>
#include <algorithm>
#include <boost/bind.hpp>
#include "../../message/chat_message.h"
#include "../partecipant/chat_partecipant.h"

class chat_room
{
public:
    void join(chat_participant_ptr participant);
    void leave(chat_participant_ptr participant);
    void deliver(const chat_message& msg);
private:
    std::set<chat_participant_ptr> participants_;
    enum { max_recent_msgs = 100 };
    chat_message_queue recent_msgs_;
};

#endif //SERVER_CHAT_ROOM_H
