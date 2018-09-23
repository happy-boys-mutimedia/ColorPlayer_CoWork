#include "audioplay_sdl2.h"
#include "ffmpeg.h"
#include <QDebug>
#include <QList>
extern "C"{
#include "SDL2/SDL.h"
}

//local define
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//Output PCM
#define OUTPUT_PCM 1

//Use SDL
#define USE_SDL 1

#define OUT_SAMPLE_RATE 44100
//-----------------------------------------------------------------------------


//local struct

//-----------------------------------------------------------------------------

//local function

//-----------------------------------------------------------------------------

//local variable

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len = 0;
static  Uint8  *audio_pos;
static  char *out_buffer;
static  Uint32 out_buffer_size;

QList<Frame *>audioDisp;
//-----------------------------------------------------------------------------

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
*/
void _SDL2_fill_audio_callback(void *udata, unsigned char *stream, int len)
{
    //qDebug()<<"_SDL2_fill_audio_callback IN len "<<len<<"audio_len "<<audio_len;
    //SDL 2.0
    SDL_memset(stream, 0, len);
    if(audio_len == 0)		/*  Only  play  if  we  have  data  left  */
    {
        return;
    }
    //qDebug()<<"callback want read len "<<len;
    len = (len > audio_len ? audio_len : len);	/*  Mix  as  much  data  as  possible  */

    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}

/* The audio function callback takes the following parameters:
 * stream: A pointer to the audio buffer to be filled
 * len: The length (in bytes) of the audio buffer
*/
int sdl2_init(void)
{
    SDL_AudioSpec wanted_spec;
    //Init
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf( "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    //Out Audio Param
    //Out Buffer Size
    out_buffer = (char *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);

    //SDL_AudioSpec
    wanted_spec.freq = XFFmpeg::Get()->sampleRate;//todo
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = XFFmpeg::Get()->channel;
    wanted_spec.silence = 0;
    wanted_spec.samples = (XFFmpeg::Get()->frame_size ? XFFmpeg::Get()->frame_size : 1024);
    wanted_spec.callback = _SDL2_fill_audio_callback;
    wanted_spec.userdata = NULL;//pCodecCtx;
    qDebug()<<"freq "<<wanted_spec.freq<<"format "<<wanted_spec.format<<"channels "<<wanted_spec.channels;
    qDebug()<<"samples "<<wanted_spec.samples;
    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        printf("can't open audio.\n");
        return -1;
    }
    qDebug()<<"sdl2_init init done !!!!!!!!";

    return 0;
}

int sdl2_display(void)
{
    SDL2AudioDisplayThread::Get()->run();
    return 0;
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
    pPI->ADispQueue.size = FRAME_QUEUE_SIZE +1;
}

void SDL2AudioDisplayThread::deinitDisplayQueue(PlayerInfo *pPI)
{
    pPI->ADispQueue.Queue->clear();
}

void SDL2AudioDisplayThread::initMasterClock(MasterClock * pMC)
{
    if (pMC != NULL)
    {
        pMasterClock = pMC;
    }
}

void SDL2AudioDisplayThread::queueMessage(MessageCmd_t MsgCmd)
{
    pMessage->message_queue(MsgCmd);
}

void SDL2AudioDisplayThread::stop()
{
    qDebug()<<"SDL2AudioDisplayThread::stop()";
    bStop = 1;
    bFirstFrame = 1;
}

void SDL2AudioDisplayThread::flush()
{
    bFirstFrame = 1;
    audio_len = 0;
    player->ADispQueue.Queue->clear();
}

void SDL2AudioDisplayThread::run()
{
    Frame *pFrame = {0};
    MessageCmd_t Msgcmd;
    Msgcmd.cmd = MESSAGE_CMD_NONE;
    Msgcmd.cmdType = MESSAGE_QUEUE_TYPES;

    threadState currentState = THREAD_NONE;
    qDebug()<<"SDL2AudioDisplayThread::run() IN";

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
            if ((pMessage->message_dequeue(&Msgcmd) == SUCCESS) && (Msgcmd.cmd == MESSAGE_CMD_RESUME))
            {
                qDebug()<<"SDL2AudioDisplayThread get resume cmd~";
                currentState = THREAD_START;
            }
            msleep(10);
            continue;
        }

        if ((pMessage->message_dequeue(&Msgcmd) == SUCCESS) && (Msgcmd.cmd == MESSAGE_CMD_PAUSE))
        {
            qDebug()<<"SDL2AudioDisplayThread get pause cmd~";
            currentState = THREAD_PAUSE;
            msleep(10);
            continue;
        }

        if (player->ADispQueue.Queue->isEmpty())
        {
            msleep(50);
            qDebug()<<"ADispQueue empty";
            continue;
        }

        player->ADispQueue.mutex.lock();
        if ((!player->ADispQueue.Queue->isEmpty()))
        {
            pFrame = player->ADispQueue.Queue->takeFirst();
        }
        player->ADispQueue.mutex.unlock();
        if (pFrame == NULL)
        {
            continue;
        }

        XFFmpeg::Get()->PutFrameToConvert(XFFmpeg::Get()->audioStreamidx, pFrame->frame);
        out_buffer_size = XFFmpeg::Get()->ToPCM(out_buffer);
        //qDebug()<<"ADispQueue get one frame!! out_buffer_size "<<out_buffer_size;
        if (out_buffer_size == 0)
        {
            qDebug()<<"ToPCM error !";
            av_frame_unref(pFrame->frame);
            pFrame->DecState = DecWait;
            pFrame->DispState = DispOver;
            continue;
        }
        while((audio_len > 0) && !bStop)//Wait until finish
            SDL_Delay(1);

        //Set audio buffer (PCM data)
        audio_chunk = (Uint8 *)out_buffer;
        //Audio buffer length
        audio_len = out_buffer_size;
        audio_pos = audio_chunk;

        if (bFirstFrame)
        {
            pMasterClock->set_clock_base(pFrame->frame->pts);
            bFirstFrame = 0;
        }
        else
        {
            pMasterClock->set_audio_clock(pFrame->frame->pts);
        }

        //Play
        SDL_PauseAudio(0);

        //显示完成更新帧存状态，解码线程可以拿帧继续解码
        pFrame->DecState = DecWait;
        pFrame->DispState = DispOver;
    }

    audio_len = 0;
    qDebug()<<"SDL2AudioDisplayThread::run() stop!";
}

SDL2AudioDisplayThread::SDL2AudioDisplayThread()
{
    player = NULL;
    bFirstFrame = 1;
    bStop = 0;

    pMessage = new message();
    if (!pMessage)
    {
        qDebug()<<"SDL2AudioDisplayThread() error\n";
    }
}

void SDL2AudioDisplayThread::init()
{
    sdl2_init();
    qDebug()<<"SDL2AudioDisplayThread::init()";
}

void SDL2AudioDisplayThread::deinit()
{
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
}

SDL2AudioDisplayThread::~SDL2AudioDisplayThread()
{
    SDL_CloseAudio();//Close SDL
    SDL_Quit();

    this->terminate();//stop run thread

    if (pMessage)
    {
        delete pMessage;
    }
    qDebug()<<"~SDL2AudioDisplayThread()";
}



