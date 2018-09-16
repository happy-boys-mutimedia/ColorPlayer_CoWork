#include "colorplayer.h"
#include "audioplay_sdl2.h"
#include "audiodecodethread.h"
#include "videodecodethread.h"
#include "demuxthread.h"
#include "videooutput.h"
#include <QDebug>
#include <QMessageBox>

ColorPlayer::ColorPlayer()
{
    player = NULL;
}

int ColorPlayer::open(const char *url)
{
    qDebug()<<"ColorPlayer::open IN";

    if (!player)
    {
        player = (PlayerInfo *)malloc(sizeof(PlayerInfo));
        if (!player)
        {
            qDebug()<<"alloc PlayerInfo fail";
            return FAILED;
        }
    }

    memset(player, 0, sizeof(PlayerInfo));
    player->bAudioFlash = 0;
    player->bVideoFlash = 0;
    player->bVideoDispFlash = 0;
    player->canReadFile = 1;

    if (!XFFmpeg::Get()->Open(url))
    {
        qDebug()<<"ffmpeg open fail";
        return FAILED;
    }

    DemuxThread::Get()->initPlayerInfo(player);
    DemuxThread::Get()->initRawQueue(player);
    VideoDecodeThread::Get()->initPlayerInfo(player);
    VideoDecodeThread::Get()->initDecodeFrameQueue(player);
    VideoOutput::Get()->initPlayerInfo(player);
    VideoOutput::Get()->initDisplayQueue(player);
    AudioDecodeThread::Get()->initPlayerInfo(player);
    AudioDecodeThread::Get()->initDecodeFrameQueue(player);
    SDL2AudioDisplayThread::Get()->initPlayerInfo(player);
    SDL2AudioDisplayThread::Get()->initDisplayQueue(player);
    SDL2AudioDisplayThread::Get()->init();

    player->isInitAll = 1;

    return SUCCESS;
}

int ColorPlayer::close()
{
    return SUCCESS;
}

int ColorPlayer::play()
{
    qDebug()<< "start thread";
    DemuxThread::Get()->start();

    AudioDecodeThread::Get()->start();
    SDL2AudioDisplayThread::Get()->start();

    VideoDecodeThread::Get()->start();
    VideoOutput::Get()->start();

    player->playerState = PLAYER_STATE_START;

    return SUCCESS;
}

int ColorPlayer::pause()
{
    MessageCmd_t MsgCmd;

    qDebug()<< "ColorPlayer send pause cmd!!";
    MsgCmd.cmd = MESSAGE_CMD_PAUSE;
    MsgCmd.cmdType = MESSAGE_CMD_QUEUE;
    SDL2AudioDisplayThread::Get()->queueMessage(MsgCmd);
    VideoOutput::Get()->queueMessage(MsgCmd);

    player->playerState = PLAYER_STATE_PAUSE;
    return SUCCESS;
}

int ColorPlayer::resume()
{
    MessageCmd_t MsgCmd;

    qDebug()<< "ColorPlayer send resume cmd!!";
    MsgCmd.cmd = MESSAGE_CMD_RESUME;
    MsgCmd.cmdType = MESSAGE_CMD_QUEUE;
    SDL2AudioDisplayThread::Get()->queueMessage(MsgCmd);
    VideoOutput::Get()->queueMessage(MsgCmd);

    player->playerState = PLAYER_STATE_RESUME;
    return SUCCESS;
}

int ColorPlayer::stop()
{
    return SUCCESS;
}

int ColorPlayer::set_pos()
{
    return SUCCESS;
}

int ColorPlayer::get_pos()
{
    return SUCCESS;
}

int ColorPlayer::cancel_seek()
{
    return SUCCESS;
}

int ColorPlayer::seek()
{
    return SUCCESS;
}

int ColorPlayer::set_speed()
{
    return SUCCESS;
}

int ColorPlayer::get_speed()
{
    return SUCCESS;
}

PlayerInfo* ColorPlayer::get_player_Instanse()
{
    return player;
}

void ColorPlayer::init_context()
{

}

void ColorPlayer::deinit_context()
{

}

ColorPlayer::~ColorPlayer()
{
}
