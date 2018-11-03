#include "audioplay_sdl2.h"
#include "ffmpeg.h"
#include <QDebug>
#include <QList>
extern "C"{
#include "SDL2/SDL.h"
}

//local define
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//-----------------------------------------------------------------------------


//local struct

//-----------------------------------------------------------------------------

//local function

//-----------------------------------------------------------------------------

//local variable
QList<Frame *>audioDisp;
QList<PCMBuffer_t *>audioPCMDisp;
//-----------------------------------------------------------------------------

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
*/
void _SDL2_fill_audio_callback(void *udata, unsigned char *stream, int len)
{
    SDL2AudioDisplayThread *pADT = (SDL2AudioDisplayThread *)udata;
    PCMBuffer_t * pPCMBuffer = NULL;

    SDL_memset(stream, 0, len);

    pADT->player->ADispPCMQueue.mutex.lock();
    if (!pADT->player->ADispPCMQueue.Queue->isEmpty())
    {
        pPCMBuffer = pADT->player->ADispPCMQueue.Queue->takeFirst();
    }
    else
    {
        //qDebug()<<"ADispPCMQueue empty";
        pADT->player->ADispPCMQueue.mutex.unlock();
        return;
    }
    pADT->player->ADispPCMQueue.mutex.unlock();

    if (pADT->bFirstFrame)
    {
        qDebug()<<"bFirstFrame pPCMBuffer->pts "<<pPCMBuffer->pts;
        pADT->pMasterClock->set_clock_base(pPCMBuffer->pts);
        pADT->bFirstFrame = 0;
        if (pADT->_funcCallback)
            pADT->_funcCallback(mediaItem_audio);
    }
    else
    {
        pADT->pMasterClock->set_audio_clock(pPCMBuffer->pts);
    }

    int buffer_size_read = 0;
    int buffer_size_index = 0;
    while (len > 0)
    {
        buffer_size_read = pPCMBuffer->bufferSize - buffer_size_index;
        if (buffer_size_read > len)
        {
            buffer_size_read = len;
        }
        if (buffer_size_read <= 0)
        {
            break;
        }

        SDL_MixAudio(stream, (const Uint8*)pPCMBuffer->bufferAddr + buffer_size_index, buffer_size_read, SDL_MIX_MAXVOLUME);
        len -= buffer_size_read;
        buffer_size_index += buffer_size_read;
    }

    pPCMBuffer->state = DISP_DONE;
}

void SDL2AudioDisplayThread::initPlayerInfo(PlayerInfo *pPI)
{
    qDebug()<<"SDL2AudioDisplayThread::initPlayerInfo";
    if (pPI != NULL)
    {
        player = pPI;
    }
}

void SDL2AudioDisplayThread::initDisplayQueue(PlayerInfo *pPI)
{
    qDebug()<<"SDL2AudioDisplayThread::initDisplayQueue";
    pPI->ADispQueue.Queue = &audioDisp;
    pPI->ADispQueue.size = FRAME_QUEUE_SIZE + 1;
    pPI->ADispPCMQueue.Queue = &audioPCMDisp;
    pPI->ADispPCMQueue.size = FRAME_QUEUE_SIZE + 1;

    for (int i = 0; i < FRAME_QUEUE_SIZE; i++)
    {
        PCMBuffers[i].bufferAddr = (char *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
        PCMBuffers[i].state = DISP_NONE;
        PCMBuffers[i].bufferSize = 0;
        PCMBuffers[i].pts = -1;
    }
}

void SDL2AudioDisplayThread::deinitDisplayQueue(PlayerInfo *pPI)
{
    if (pPI)
    {
        pPI->ADispQueue.Queue->clear();
        pPI->ADispPCMQueue.Queue->clear();
        for (int i = 0; i < FRAME_QUEUE_SIZE; i++)
        {
            if (!PCMBuffers[i].bufferAddr)
            {
                av_free((void *)PCMBuffers[i].bufferAddr);
                PCMBuffers[i].bufferAddr = NULL;
            }
            PCMBuffers[i].state = DISP_NONE;
            PCMBuffers[i].bufferSize = 0;
            PCMBuffers[i].pts = -1;
        }
    }
}

void SDL2AudioDisplayThread::initMasterClock(MasterClock * pMC)
{
    if (pMC != NULL)
    {
        pMasterClock = pMC;
    }
}

PCMBuffer_t *SDL2AudioDisplayThread::GetOneValidPCMBuffer()
{
    PCMBuffer_t *pPCMBuffer = NULL;
    int i;

    for (i = 0; i < FRAME_QUEUE_SIZE; i++)
    {
        if (PCMBuffers[i].state != DISP_WAIT && PCMBuffers[i].bufferAddr != NULL)
        {
            pPCMBuffer = &PCMBuffers[i];
            break;
        }
    }

    if (i == FRAME_QUEUE_SIZE)
    {
        //qDebug()<<" can't find valid PCMBuffers for SDL2";
        return NULL;
    }

    return pPCMBuffer;
}

void SDL2AudioDisplayThread::queueMessage(MessageCmd_t MsgCmd)
{
    pMessage->message_queue(MsgCmd);
}

void SDL2AudioDisplayThread::setCallback(pFuncCallback callback)
{
    if (callback)
        _funcCallback = callback;
}

void SDL2AudioDisplayThread::stop()
{
    bStop = 1;
    bFirstFrame = 1;
    if (player)
        player->pWaitCondAudioOutputThread->wakeAll();

    wait();
    qDebug()<<"SDL2AudioDisplayThread::stop  wait done";
}

void SDL2AudioDisplayThread::flush()
{
    bFirstFrame = 1;
    player->ADispQueue.Queue->clear();
    player->ADispPCMQueue.Queue->clear();

    for (int i = 0; i < FRAME_QUEUE_SIZE; i++)
    {
        PCMBuffers[i].state = DISP_NONE;
        PCMBuffers[i].bufferSize = 0;
        PCMBuffers[i].pts = -1;
    }
}

void SDL2AudioDisplayThread::run()
{
    Frame *pFrame = NULL;
    int bEof = 0;
    PCMBuffer_t *pPCMBuffer = NULL;
    MessageCmd_t Msgcmd;
    Msgcmd.cmd = MESSAGE_CMD_NONE;
    Msgcmd.cmdType = MESSAGE_QUEUE_TYPES;

    threadState currentState = THREAD_NONE;
    qDebug()<<"SDL2AudioDisplayThread::run() IN";
    SDL_PauseAudio(0);

    bStop = 0;
    while(!bStop)
    {
        if (!player)
        {
            msleep(100);
            continue;
        }

        if (currentState == THREAD_PAUSE)
        {
            if (pMessage->message_dequeue(&Msgcmd) == SUCCESS)
            {
                if (Msgcmd.cmd == MESSAGE_CMD_RESUME)
                {
                    qDebug()<<"SDL2AudioDisplayThread get resume cmd~";
                    currentState = THREAD_START;
                    SDL_PauseAudio(0);
                    continue;
                }
                else if (Msgcmd.cmd == MESSAGE_CMD_SEEK)
                {
                    qDebug()<<"SDL2AudioDisplayThread get seek cmd~";
                    currentState = THREAD_SEEK;
                    SDL_PauseAudio(0);
                    continue;
                }
            }
            msleep(100);
            continue;
        }

        if (pMessage->message_dequeue(&Msgcmd) == SUCCESS)
        {
            if (Msgcmd.cmd == MESSAGE_CMD_PAUSE)
            {
                qDebug()<<"SDL2AudioDisplayThread get pause cmd~";
                currentState = THREAD_PAUSE;
                SDL_PauseAudio(1);
                continue;
            }
            else if (Msgcmd.cmd == MESSAGE_CMD_FORCE_EOF)
            {
                qDebug()<<"SDL2AudioDisplayThread get EOF cmd~";
                bEof = 1;
            }
        }

        if (!(pPCMBuffer = GetOneValidPCMBuffer()))
        {
            //qDebug()<<"GetOneValidPCMBuffer empty";
            msleep(50);
            continue; 
        }

        player->ADispQueue.mutex.lock();
        if (player->ADispQueue.Queue->isEmpty())
        {
            if (bEof)
            {
                bStop = 1;
                player->ADispQueue.mutex.unlock();
                continue;
            }
            else if (!bStop)
            {
                //qDebug()<<"ADispQueue empty ==> wait";
                player->pWaitCondAudioOutputThread->wait(&(player->ADispQueue.mutex));
                player->ADispQueue.mutex.unlock();
                //qDebug()<<" pWaitCondAudioOutputThread Queue ==> wait ==>wakeup";
                continue;
            }
        }
        else
        {
            pFrame = player->ADispQueue.Queue->takeFirst();
        }
        player->ADispQueue.mutex.unlock();

        if (pFrame == NULL)
        {
            continue;
        }

        XFFmpeg::Get()->PutFrameToConvert(XFFmpeg::Get()->audioStreamidx, pFrame->frame);
        pPCMBuffer->bufferSize = XFFmpeg::Get()->ToPCM(pPCMBuffer->bufferAddr);
        if (pPCMBuffer->bufferSize == 0)
        {
            qDebug()<<"ToPCM error !";
            av_frame_unref(pFrame->frame);
            pFrame->DecState = DecWait;
            pFrame->DispState = DispOver;
            pPCMBuffer->state = DISP_DONE;
            player->pWaitCondAudioDecodeThread->wakeAll();
            continue;
        }

        pPCMBuffer->pts = pFrame->frame->pts;
        pFrame->DecState = DecWait;
        pFrame->DispState = DispOver;
        player->pWaitCondAudioDecodeThread->wakeAll();

        player->ADispPCMQueue.mutex.lock();
        pPCMBuffer->state = DISP_WAIT;
        //qDebug()<<"ADispPCMQueue ==> append";
        player->ADispPCMQueue.Queue->append(pPCMBuffer);
        player->ADispPCMQueue.mutex.unlock();
    }
    pMessage->message_clear();
    qDebug()<<"SDL2AudioDisplayThread::run() stop!";
}

SDL2AudioDisplayThread::SDL2AudioDisplayThread()
{
    player = NULL;
    bFirstFrame = 1;
    bStop = 0;
    _funcCallback = NULL;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"SDL2AudioDisplayThread() error\n";
    }
}

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
*/
void SDL2AudioDisplayThread::init()
{
    SDL_AudioSpec wanted_spec;
    //Init
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return;
    }

    //SDL_AudioSpec
    wanted_spec.freq = XFFmpeg::Get()->sampleRate;//todo
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = XFFmpeg::Get()->channel;
    wanted_spec.silence = 0;
    wanted_spec.samples = (XFFmpeg::Get()->frame_size ? XFFmpeg::Get()->frame_size : 1536);//ac-3 Dolby digital:1536
    wanted_spec.callback = _SDL2_fill_audio_callback;
    wanted_spec.userdata = (void *)this;
    qDebug()<<"freq "<<wanted_spec.freq<<"format "<<wanted_spec.format<<"channels "<<wanted_spec.channels;
    qDebug()<<"samples "<<wanted_spec.samples;
    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        printf("can't open audio.\n");
        return;
    }

    //Play
    SDL_PauseAudio(0);

    qDebug()<<"sdl2_init init done !!!!!!!!";

    return;
}

void SDL2AudioDisplayThread::deinit()
{
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
}

SDL2AudioDisplayThread::~SDL2AudioDisplayThread()
{
    qDebug()<<"~SDL2AudioDisplayThread() IN";
    SDL_CloseAudio();//Close SDL
    SDL_Quit();

    stop();

    if (pMessage)
    {
        delete pMessage;
    }

    for (int i = 0; i < FRAME_QUEUE_SIZE; i++)
    {
        if (!PCMBuffers[i].bufferAddr)
        {
            av_free((void *)PCMBuffers[i].bufferAddr);
            PCMBuffers[i].bufferAddr = NULL;
        }
        PCMBuffers[i].state = DISP_NONE;
        PCMBuffers[i].bufferSize = 0;
        PCMBuffers[i].pts = -1;
    }
}



