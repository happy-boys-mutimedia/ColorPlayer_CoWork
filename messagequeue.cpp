#include "messagequeue.h"
#include "common.h"
#include <QList>
#include <QDebug>

message::message()
{
    Queue = new QList<MessageCmd_t>();
    if (Queue == NULL)
    {
        qDebug()<<"Message() error\n";
    }
}

message::~message()
{
    if (Queue != NULL)
    {
        Queue->clear();
        delete Queue;
    }
}

int message::message_queue(MessageCmd_t cmd)
{
    mutex.lock();
    Queue->append(cmd);
    mutex.unlock();

    return SUCCESS;
}

int message::message_dequeue(MessageCmd_t *cmd)
{
    mutex.lock();
    if (!Queue->isEmpty())
    {
        *cmd = Queue->takeFirst();
    }
    else
    {
        mutex.unlock();
        return FAILED;
    }
    mutex.unlock();

    return SUCCESS;
}

int message::message_cmd_size()
{
    return Queue->count();
}

int message::message_is_empty()
{
    return Queue->isEmpty();
}

void message::message_clear()
{
    mutex.lock();
    Queue->clear();
    mutex.unlock();
}

