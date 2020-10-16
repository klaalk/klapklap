//
// Created by Klaus on 06/05/2019.
//

#include "kk_task.h"
#include <QDebug>

KKTask::KKTask(std::function<void()> f): function(f){}

// When the thread pool kicks up
// it's going to hit this run, and it's going to do this time consuming task.
// After it's done, we're going to send the results back to our main thread.
// This runs in the separate thread, and we do not have any control over this thread,
// but Qt does.
// This may just stay in the queue for several ms depending on how busy the server is.
void KKTask::run()
{
    // time consumer
    function();
}
