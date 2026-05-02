#ifndef SoundSystemAL_H__
#define SoundSystemAL_H__

#include "SoundSystem.h"

#if defined(_WIN32)
    typedef char ALchar;
    typedef unsigned char ALboolean;
    typedef int ALenum;
    typedef int ALint;
    typedef int ALsizei;
    typedef float ALfloat;
    typedef void ALvoid;
    typedef unsigned int ALuint;

    typedef char ALCchar;
    typedef unsigned char ALCboolean;
    typedef int ALCenum;
    typedef int ALCint;

    struct ALCdevice_struct;
    struct ALCcontext_struct;
    typedef ALCdevice_struct ALCdevice;
    typedef ALCcontext_struct ALCcontext;
#elif defined(__APPLE__)
    #include <OpenAL/al.h>
    #include <OpenAL/alc.h>
#else
    #include <AL/al.h>
    #include <AL/alc.h>
#endif

#include <vector>
#include <list>

//
// NOTE: This class is only the core OpenAL part of the sound engine.
//       Some audio setup code can still be managed from respective app
//       setup code (e.g. the main app delegate for iOS).
//

class SoundSystemAL: public SoundSystem
{
	//typedef std::list<SLObjectItf> SoundList;
public:
    SoundSystemAL();
	~SoundSystemAL();

	virtual bool isAvailable() { return available; }
	virtual void init();
	virtual void destroy();

    virtual void enable(bool status);
    
    virtual void setListenerPos(float x, float y, float z);
	virtual void setListenerAngle(float deg);

	virtual void load(const std::string& name){}
    virtual void play(const std::string& name){}
    virtual void pause(const std::string& name){}
    virtual void stop(const std::string& name){}
	virtual void playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch);
	bool playMusicWavFile(const std::string& path, float volume);
	void stopMusic();
	void setMusicVolume(float volume);
	bool isMusicPlaying() const;

private:
    class Buffer {
    public:
        Buffer()
        :   inited(false)
        {}
        bool inited;
        ALuint bufferID;
        char* framePtr;
    };
    
	void removeStoppedSounds();

    static const int MaxNumSources = 12;

	//SoundList playingBuffers;

    Vec3 _listenerPos;
    float _rotation;
    
	bool available;
    
    ALCcontext* context;
	ALCdevice* device;
	ALuint _musicSource;
	ALuint _musicBuffer;
    
    ALuint _sources[MaxNumSources];
    std::vector<Buffer> _buffers;

    bool getFreeSourceIndex(int* src);
    bool getBufferId(const SoundDesc& sound, ALuint* buf);
    
public:
};

#endif /*SoundSystemAL_H__ */
