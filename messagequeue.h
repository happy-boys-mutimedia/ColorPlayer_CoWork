#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <QMutex>
#include <QList>

enum {
    MESSAGE_CMD_NONE,
    MESSAGE_CMD_PAUSE,
    MESSAGE_CMD_RESUME,
    MESSAGE_CMD_SEEK,
    MESSAGE_CMD_STOP,
    MESSAGE_CMD_SEL_TRACK,
    MESSAGE_CMD_FLUSH,
    MESSAGE_CMD_DECODE,
    MESSAGE_CMD_DMX,
    MESSAGE_CMD_WAIT_RBUF,
    MESSAGE_CMD_REINIT,
    MESSAGE_CMD_RESYNC,
    MESSAGE_CMD_REINIT_AUD_DRV,
    MESSAGE_CMD_RENDER,
    MESSAGE_CMD_INPUT_TYPE_CHANGE,
    MESSAGE_CMD_FORCE_EOF,
    MESSAGE_CMD_CHANGE_SPEED,
    MESSAGE_CMD_NEED_AVSYNC,
    MESSAGE_CMD_WINDOW_MINMiZED,
    MESSAGE_CMD_WINDOW_RESUME,
    MESSAGE_CMD_MULTIPLE_PLAY,
    MESSAGE_CMD_CANCEL_AVSYNC
};

typedef enum
{
    MESSAGE_CMD_QUEUE = 0,
    MESSAGE_NOTIFY_QUEUE,
    MESSAGE_QUEUE_TYPES
} MessageQueueType_e;

typedef struct
{
    MessageQueueType_e cmdType;
    int cmd;
} MessageCmd_t;

class message
{
public:
    message();
    virtual ~message();

    int message_queue(MessageCmd_t cmd);
    int message_dequeue(MessageCmd_t *cmd);
    int message_cmd_size();
    int message_is_empty();
    void message_clear();
private:
    QMutex mutex;
    QList<MessageCmd_t> *Queue;
};


#endif // MESSAGEQUEUE_H
