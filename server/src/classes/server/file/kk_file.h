//
// Created by Klaus on 06/05/2019.
//

#ifndef KK_FILE_H
#define KK_FILE_H

#include <set>
#include <iostream>
#include <algorithm>
#include <memory>
#include "../../../../../libs/src/classes/payload/kk_payload.h"
#include "../participant/kk_participant.h"


class kk_file {
public:
    void join(std::shared_ptr<kk_participant> participant);

    void leave(std::shared_ptr<kk_participant> participant);

    void deliver(const kk_payload &msg);

private:
    std::set<std::shared_ptr<kk_participant>> participants_;
    enum {
        max_recent_msgs = 100
    };
    kk_payload_queue recent_msgs_;
};

typedef std::shared_ptr<kk_file> kk_file_ptr;
typedef std::shared_ptr<QMap<QString, kk_file_ptr>> map_files_ptr;
#endif //KK_FILE_H
