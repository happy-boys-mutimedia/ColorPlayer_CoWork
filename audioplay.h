#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H


class AudioPlay
{
public:
    static AudioPlay *Get();
    virtual bool Start()=0;
    virtual bool Stop() = 0;
    virtual void Play(bool isplay)=0;
    virtual bool Write(const char *data, int datasize )=0;
    virtual int GetFree() = 0;
    virtual void Release() = 0;
    int sampleRate = 48000;
    int sampleSize = 16;
    int channel = 2;
    virtual ~AudioPlay();
protected:
    AudioPlay();
};

#endif // AUDIOPLAY_H
