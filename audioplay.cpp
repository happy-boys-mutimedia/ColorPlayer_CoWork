#include "audioplay.h"
#include "AudioPlay.h"
#include <QAudioOutput>
#include <QDebug>

class CXAudioPlay :public AudioPlay
{
public:
    virtual ~CXAudioPlay();
    QAudioOutput *output = NULL;
    QIODevice *io = NULL;
    QMutex mutex;
    bool Stop()
    {
        mutex.lock();
        if (output)
        {
            output->stop();
            delete output;
            output = NULL;
            io = NULL;
        }
        mutex.unlock();
        return true;
    }

    bool Start()
    {
        Stop();
        mutex.lock();
        QAudioFormat fmt;
        fmt.setSampleRate(this->sampleRate);
        fmt.setSampleSize(this->sampleSize);
        fmt.setSampleType(QAudioFormat::UnSignedInt);
        fmt.setChannelCount(this->channel);
        fmt.setCodec("audio/pcm");
        fmt.setByteOrder(QAudioFormat::LittleEndian);

        output = new QAudioOutput(fmt);
        io = output->start();
        mutex.unlock();
        return true;
    }

    void Play(bool isplay)
    {
        mutex.lock();
        if (!output)
        {
            mutex.unlock();
            return;
        }

        if (isplay)
        {
            output->resume();
        }
        else
        {
            output->suspend();
        }
        mutex.unlock();
        return;
    }

    bool Write(const char *data, int datasize )
    {
        mutex.lock();
        if (io)
        io->write(data, datasize);
        mutex.unlock();
        return true;
    }

    int GetFree()
    {
        mutex.lock();
        if (!output)
        {
            mutex.unlock();
            return 0;
        }
        int free = output->bytesFree();
        mutex.unlock();
        return free;
    }

    void Release()
    {
        qDebug()<<"AudioPlay::~AudioPlay()";
        mutex.lock();
        if (output)
        {
            output->stop();
            delete output;
            output = NULL;
            io = NULL;
        }
        mutex.unlock();
    }

};

CXAudioPlay::~CXAudioPlay()
{
    CXAudioPlay::Release();
}

AudioPlay::AudioPlay()
{
}


AudioPlay::~AudioPlay()
{
}

AudioPlay *AudioPlay::Get()
{
    static CXAudioPlay ap;
    return &ap;
}
