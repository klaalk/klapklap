//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_FILE_H
#define KK_FILE_H

#include <set>
#include <iostream>
#include <algorithm>
#include <boost/bind.hpp>
#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../partecipant/kk_partecipant.h"


class kk_file {
public:
    void join(kk_participant_ptr participant);

    void leave(kk_participant_ptr participant);

    void deliver(const kk_payload &msg);

private:
    std::set<kk_participant_ptr> participants_;
    enum {
        max_recent_msgs = 100
    };
    kk_kk_payload_queue recent_msgs_;
};

#endif //KK_FILE_H
