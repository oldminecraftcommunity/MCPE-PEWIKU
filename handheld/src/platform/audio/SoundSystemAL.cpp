//#include "ios/OpenALSupport.h"
#include "SoundSystemAL.h"
#include "../../util/Mth.h"
#include "../../world/level/tile/Tile.h"
#include "../../world/phys/Vec3.h"
#include "../../client/sound/Sound.h"

#include "../log.h"
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

static const ALenum AL_NO_ERROR = 0;
static const ALenum AL_FALSE = 0;
static const ALenum AL_SOURCE_STATE = 0x1010;
static const ALenum AL_PLAYING = 0x1012;
static const ALenum AL_LOOPING = 0x1007;
static const ALenum AL_BUFFER = 0x1009;
static const ALenum AL_GAIN = 0x100A;
static const ALenum AL_POSITION = 0x1004;
static const ALenum AL_VELOCITY = 0x1006;
static const ALenum AL_ORIENTATION = 0x100F;
static const ALenum AL_PITCH = 0x1003;
static const ALenum AL_ROLLOFF_FACTOR = 0x1021;
static const ALenum AL_LINEAR_DISTANCE_CLAMPED = 0xD004;
static const ALenum AL_FORMAT_MONO8 = 0x1100;
static const ALenum AL_FORMAT_MONO16 = 0x1101;
static const ALenum AL_FORMAT_STEREO8 = 0x1102;
static const ALenum AL_FORMAT_STEREO16 = 0x1103;

typedef ALenum (__cdecl* alGetErrorProc)();
typedef void (__cdecl* alDistanceModelProc)(ALenum value);
typedef void (__cdecl* alGenSourcesProc)(ALsizei n, ALuint* sources);
typedef void (__cdecl* alDeleteSourcesProc)(ALsizei n, const ALuint* sources);
typedef void (__cdecl* alSourcefProc)(ALuint sid, ALenum param, ALfloat value);
typedef void (__cdecl* alSourceiProc)(ALuint sid, ALenum param, ALint value);
typedef void (__cdecl* alSource3fProc)(ALuint sid, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
typedef void (__cdecl* alSourcePlayProc)(ALuint sid);
typedef void (__cdecl* alSourceStopProc)(ALuint sid);
typedef void (__cdecl* alGetSourceiProc)(ALuint sid, ALenum param, ALint* value);
typedef void (__cdecl* alListenerfvProc)(ALenum param, const ALfloat* values);
typedef void (__cdecl* alListener3fProc)(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3);
typedef void (__cdecl* alGenBuffersProc)(ALsizei n, ALuint* buffers);
typedef void (__cdecl* alDeleteBuffersProc)(ALsizei n, const ALuint* buffers);
typedef void (__cdecl* alBufferDataProc)(ALuint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq);

typedef ALCdevice* (__cdecl* alcOpenDeviceProc)(const ALCchar* devicename);
typedef ALCboolean (__cdecl* alcCloseDeviceProc)(ALCdevice* device);
typedef ALCcontext* (__cdecl* alcCreateContextProc)(ALCdevice* device, const ALCint* attrlist);
typedef ALCboolean (__cdecl* alcMakeContextCurrentProc)(ALCcontext* context);
typedef void (__cdecl* alcDestroyContextProc)(ALCcontext* context);

static HMODULE g_openAlModule = NULL;
static alGetErrorProc p_alGetError = NULL;
static alDistanceModelProc p_alDistanceModel = NULL;
static alGenSourcesProc p_alGenSources = NULL;
static alDeleteSourcesProc p_alDeleteSources = NULL;
static alSourcefProc p_alSourcef = NULL;
static alSourceiProc p_alSourcei = NULL;
static alSource3fProc p_alSource3f = NULL;
static alSourcePlayProc p_alSourcePlay = NULL;
static alSourceStopProc p_alSourceStop = NULL;
static alGetSourceiProc p_alGetSourcei = NULL;
static alListenerfvProc p_alListenerfv = NULL;
static alListener3fProc p_alListener3f = NULL;
static alGenBuffersProc p_alGenBuffers = NULL;
static alDeleteBuffersProc p_alDeleteBuffers = NULL;
static alBufferDataProc p_alBufferData = NULL;
static alcOpenDeviceProc p_alcOpenDevice = NULL;
static alcCloseDeviceProc p_alcCloseDevice = NULL;
static alcCreateContextProc p_alcCreateContext = NULL;
static alcMakeContextCurrentProc p_alcMakeContextCurrent = NULL;
static alcDestroyContextProc p_alcDestroyContext = NULL;

template <typename T>
static bool loadAlProc(T& target, const char* name)
{
    target = (T)GetProcAddress(g_openAlModule, name);
    return target != NULL;
}

static bool ensureOpenALLoaded()
{
    if (g_openAlModule) {
        return true;
    }

    g_openAlModule = LoadLibraryA("OpenAL32.dll");
    if (!g_openAlModule) {
        return false;
    }

    bool ok =
        loadAlProc(p_alGetError, "alGetError") &&
        loadAlProc(p_alDistanceModel, "alDistanceModel") &&
        loadAlProc(p_alGenSources, "alGenSources") &&
        loadAlProc(p_alDeleteSources, "alDeleteSources") &&
        loadAlProc(p_alSourcef, "alSourcef") &&
        loadAlProc(p_alSourcei, "alSourcei") &&
        loadAlProc(p_alSource3f, "alSource3f") &&
        loadAlProc(p_alSourcePlay, "alSourcePlay") &&
        loadAlProc(p_alSourceStop, "alSourceStop") &&
        loadAlProc(p_alGetSourcei, "alGetSourcei") &&
        loadAlProc(p_alListenerfv, "alListenerfv") &&
        loadAlProc(p_alListener3f, "alListener3f") &&
        loadAlProc(p_alGenBuffers, "alGenBuffers") &&
        loadAlProc(p_alDeleteBuffers, "alDeleteBuffers") &&
        loadAlProc(p_alBufferData, "alBufferData") &&
        loadAlProc(p_alcOpenDevice, "alcOpenDevice") &&
        loadAlProc(p_alcCloseDevice, "alcCloseDevice") &&
        loadAlProc(p_alcCreateContext, "alcCreateContext") &&
        loadAlProc(p_alcMakeContextCurrent, "alcMakeContextCurrent") &&
        loadAlProc(p_alcDestroyContext, "alcDestroyContext");

    if (!ok) {
        FreeLibrary(g_openAlModule);
        g_openAlModule = NULL;
        p_alGetError = NULL;
        p_alDistanceModel = NULL;
        p_alGenSources = NULL;
        p_alDeleteSources = NULL;
        p_alSourcef = NULL;
        p_alSourcei = NULL;
        p_alSource3f = NULL;
        p_alSourcePlay = NULL;
        p_alSourceStop = NULL;
        p_alGetSourcei = NULL;
        p_alListenerfv = NULL;
        p_alListener3f = NULL;
        p_alGenBuffers = NULL;
        p_alDeleteBuffers = NULL;
        p_alBufferData = NULL;
        p_alcOpenDevice = NULL;
        p_alcCloseDevice = NULL;
        p_alcCreateContext = NULL;
        p_alcMakeContextCurrent = NULL;
        p_alcDestroyContext = NULL;
        return false;
    }

    return true;
}

#define alGetError p_alGetError
#define alDistanceModel p_alDistanceModel
#define alGenSources p_alGenSources
#define alDeleteSources p_alDeleteSources
#define alSourcef p_alSourcef
#define alSourcei p_alSourcei
#define alSource3f p_alSource3f
#define alSourcePlay p_alSourcePlay
#define alSourceStop p_alSourceStop
#define alGetSourcei p_alGetSourcei
#define alListenerfv p_alListenerfv
#define alListener3f p_alListener3f
#define alGenBuffers p_alGenBuffers
#define alDeleteBuffers p_alDeleteBuffers
#define alBufferData p_alBufferData
#define alcOpenDevice p_alcOpenDevice
#define alcCloseDevice p_alcCloseDevice
#define alcCreateContext p_alcCreateContext
#define alcMakeContextCurrent p_alcMakeContextCurrent
#define alcDestroyContext p_alcDestroyContext
#endif

static const char* errIdString = 0;

static unsigned short _readLE16(const unsigned char* p)
{
    return (unsigned short)((unsigned short)p[0] | ((unsigned short)p[1] << 8));
}

static unsigned int _readLE32(const unsigned char* p)
{
    return (unsigned int)p[0]
         | ((unsigned int)p[1] << 8)
         | ((unsigned int)p[2] << 16)
         | ((unsigned int)p[3] << 24);
}

static bool _loadWavPcm(const std::string& path, std::vector<char>& pcm, int& channels, int& bitsPerSample, int& sampleRate)
{
    pcm.clear();
    channels = 0;
    bitsPerSample = 0;
    sampleRate = 0;

    FILE* fp = std::fopen(path.c_str(), "rb");
    if (!fp) {
        return false;
    }

    unsigned char header[12];
    if (std::fread(header, 1, sizeof(header), fp) != sizeof(header)) {
        std::fclose(fp);
        return false;
    }

    if (std::memcmp(header + 0, "RIFF", 4) != 0 || std::memcmp(header + 8, "WAVE", 4) != 0) {
        std::fclose(fp);
        return false;
    }

    bool hasFmt = false;
    bool hasData = false;

    while (!hasData) {
        unsigned char chunkHeader[8];
        if (std::fread(chunkHeader, 1, sizeof(chunkHeader), fp) != sizeof(chunkHeader)) {
            break;
        }

        const unsigned int chunkSize = _readLE32(chunkHeader + 4);
        if (std::memcmp(chunkHeader + 0, "fmt ", 4) == 0) {
            if (chunkSize < 16) {
                break;
            }

            std::vector<unsigned char> fmt(chunkSize);
            if (std::fread(&fmt[0], 1, chunkSize, fp) != chunkSize) {
                break;
            }

            const unsigned short formatTag = _readLE16(&fmt[0]);
            const unsigned short ch = _readLE16(&fmt[2]);
            const unsigned int rate = _readLE32(&fmt[4]);
            const unsigned short bits = _readLE16(&fmt[14]);

            if (formatTag != 1) {
                break;
            }
            if ((ch != 1 && ch != 2) || (bits != 8 && bits != 16) || rate == 0) {
                break;
            }

            channels = (int)ch;
            bitsPerSample = (int)bits;
            sampleRate = (int)rate;
            hasFmt = true;
        } else if (std::memcmp(chunkHeader + 0, "data", 4) == 0) {
            if (!hasFmt || chunkSize == 0) {
                break;
            }

            pcm.resize(chunkSize);
            if (std::fread(&pcm[0], 1, chunkSize, fp) != chunkSize) {
                pcm.clear();
                break;
            }
            hasData = true;
        } else {
            if (std::fseek(fp, (long)chunkSize, SEEK_CUR) != 0) {
                break;
            }
        }

        if (chunkSize & 1) {
            if (std::fseek(fp, 1, SEEK_CUR) != 0) {
                break;
            }
        }
    }

    std::fclose(fp);
    return hasFmt && hasData;
}

void checkError() {
#ifdef _WIN32
    if (!alGetError) return;
#endif

    while (1) {
        ALenum err = alGetError();
        if(err == AL_NO_ERROR) return;

        LOGI("### SoundSystemAL error: %d ####: %s\n", err, errIdString==0?"(none)":errIdString);
    }
}

//typedef ALvoid	AL_APIENTRY	(*alBufferDataStaticProcPtr) (const ALint bid, ALenum format, ALvoid *data, ALsizei size, ALsizei freq);
//ALvoid alBufferDataStaticProc(const ALint bid, ALenum format, ALvoid* data, ALsizei size, ALsizei freq)
//{
//	static alBufferDataStaticProcPtr proc = NULL;
//    
//    if (proc == NULL) {
//        proc = (alBufferDataStaticProcPtr) alcGetProcAddress(NULL, (const ALCchar*) "alBufferDataStatic");
//    }
//
//    if (proc)
//        proc(bid, format, data, size, freq);
//	
//    return;
//}
//
SoundSystemAL::SoundSystemAL()
:	available(true),
    context(0),
    device(0),
    _musicSource(0),
    _musicBuffer(0),
    _rotation(-9999.9f)
{
    _buffers.reserve(64);
	init();
}

SoundSystemAL::~SoundSystemAL()
{
    if (!available) return;

    alDeleteSources(MaxNumSources, _sources);

    for (int i = 0; i < (int)_buffers.size(); ++i)
        if (_buffers[i].inited) alDeleteBuffers(1, &_buffers[i].bufferID);

    stopMusic();
    if (_musicSource) {
        alDeleteSources(1, &_musicSource);
        _musicSource = 0;
    }
    if (_musicBuffer) {
        alDeleteBuffers(1, &_musicBuffer);
        _musicBuffer = 0;
    }

    alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	
	// Close the device
	alcCloseDevice(device);
}

void SoundSystemAL::init()
{
#ifdef _WIN32
    if (!ensureOpenALLoaded()) {
        available = false;
        return;
    }
#endif

	device = alcOpenDevice(NULL);
	if(device) {
		context = alcCreateContext(device, NULL);
		alcMakeContextCurrent(context);
        
        alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);
        
        alGenSources(MaxNumSources, _sources);
		for(int index = 0; index < MaxNumSources; index++) {
            ALuint sourceID = _sources[index];
            
            alSourcef(sourceID, AL_ROLLOFF_FACTOR, 0.0f);
		}

        alGenSources(1, &_musicSource);
        if (_musicSource) {
            alSourcef(_musicSource, AL_ROLLOFF_FACTOR, 0.0f);
            alSource3f(_musicSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
            alSourcei(_musicSource, AL_LOOPING, AL_FALSE);
        }
        
		float listenerPos[] = {0, 0, 0};
		float listenerOri[] = {0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
		float listenerVel[] = {0, 0, 0};
		alListenerfv(AL_POSITION, listenerPos);
		alListenerfv(AL_ORIENTATION, listenerOri);
		alListenerfv(AL_VELOCITY, listenerVel);
        
        errIdString = "Init audio";
        checkError();
	}
    else {
        available = false;
    }
}

void SoundSystemAL::enable(bool status) {
    if (!available) return;
    LOGI("Enabling? audio: %d (context %p)\n", status, context);
    if (status) {
        alcMakeContextCurrent(context);
        errIdString = "Enable audio";
    }
    else {
        alcMakeContextCurrent(NULL);
        errIdString = "Disable audio";
    }

    checkError();
}

void SoundSystemAL::destroy() {}

void SoundSystemAL::setListenerPos( float x, float y, float z )
{
    if (!available) return;
    // Note: listener position is thought to be 0,0,0 now
    
    /*
    if (_listenerPos.x != x || _listenerPos.y != y || _listenerPos.z != z) {
        _listenerPos.set(x, y, z);
        alListener3f(AL_POSITION, x, y, z);
        
        static int _n = 0;
        if (++_n == 20) {
            _n = 0;
            LOGI("Setting position for listener: %f, %f, %f\n", _listenerPos.x, _listenerPos.y, _listenerPos.z);
        }
    }
  */
}

void SoundSystemAL::setListenerAngle( float deg )
{
    if (!available) return;
    if (_rotation != deg) {
        _rotation = deg;

        float rad = deg * Mth::DEGRAD;

        static ALfloat orientation[] = {0, 0, 0,    0, 1, 0};
        orientation[0] = -Mth::sin( rad );
        orientation[2] =  Mth::cos( rad );
        alListenerfv(AL_ORIENTATION, orientation);
    }
}

void SoundSystemAL::playAt( const SoundDesc& sound, float x, float y, float z, float volume, float pitch )
{
    if (!available) return;
    if (pitch < 0.01f) pitch = 1;

    //LOGI("playing sound '%s' with volume/pitch: %f, %f @ %f, %f, %f\n", sound.name.c_str(), volume, pitch, x, y, z);
    
    ALuint bufferID;
    if (!getBufferId(sound, &bufferID)) {
        errIdString = "Get buffer (failed)";
        checkError();
        LOGE("getBufferId returned false!\n");
        return;
    }
    errIdString = "Get buffer";
    checkError();
    //LOGI("playing sound %d - '%s' with volume/pitch: %f, %f @ %f, %f, %f\n", bufferID, sound.name.c_str(), volume, pitch, x, y, z);

    int sourceIndex;
    errIdString = "Get free index";
    if (!getFreeSourceIndex(&sourceIndex)) {
        LOGI("No free sound sources left @ SoundSystemAL::playAt\n");
        return;
    }

    ALuint sourceID = _sources[sourceIndex];
    checkError();

    alSourcei(sourceID, AL_BUFFER, 0);
    errIdString = "unbind";
    checkError();
  	alSourcei(sourceID, AL_BUFFER, bufferID);
    errIdString = "bind";
    checkError();

    alSourcef(sourceID, AL_PITCH, pitch);
    errIdString = "pitch";
    checkError();
	alSourcef(sourceID, AL_GAIN, volume);
    errIdString = "gain";
    checkError();
	
    alSourcei(sourceID, AL_LOOPING, AL_FALSE);
    errIdString = "looping";
    checkError();
	alSource3f(sourceID, AL_POSITION, x, y, z);
    errIdString = "position";
    checkError();
	
	alSourcePlay(sourceID);
    errIdString = "source play";

    checkError();
}

bool SoundSystemAL::playMusicWavFile(const std::string& path, float volume)
{
    if (!available || !_musicSource) return false;

    std::vector<char> pcm;
    int channels = 0;
    int bitsPerSample = 0;
    int sampleRate = 0;
    if (!_loadWavPcm(path, pcm, channels, bitsPerSample, sampleRate)) {
        return false;
    }

    stopMusic();

    if (!_musicBuffer) {
        alGenBuffers(1, &_musicBuffer);
    }
    if (!_musicBuffer) {
        return false;
    }

    ALenum format = AL_FORMAT_MONO16;
    if (bitsPerSample == 8) {
        format = channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
    } else {
        format = channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    }

    alBufferData(_musicBuffer, format, &pcm[0], (ALsizei)pcm.size(), sampleRate);
    errIdString = "music buffer";
    checkError();

    alSourcei(_musicSource, AL_BUFFER, 0);
    alSourcei(_musicSource, AL_BUFFER, (ALint)_musicBuffer);
    alSourcef(_musicSource, AL_GAIN, volume);
    alSourcef(_musicSource, AL_PITCH, 1.0f);
    alSourcei(_musicSource, AL_LOOPING, AL_FALSE);
    alSourcePlay(_musicSource);
    errIdString = "music play";
    checkError();
    return true;
}

void SoundSystemAL::stopMusic()
{
    if (!available || !_musicSource) return;

    alSourceStop(_musicSource);
    alSourcei(_musicSource, AL_BUFFER, 0);
}

void SoundSystemAL::setMusicVolume(float volume)
{
    if (!available || !_musicSource) return;
    alSourcef(_musicSource, AL_GAIN, volume);
}

bool SoundSystemAL::isMusicPlaying() const
{
    if (!available || !_musicSource) return false;

    ALint state = 0;
    alGetSourcei(_musicSource, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

/*static*/
void SoundSystemAL::removeStoppedSounds()
{
}

bool SoundSystemAL::getFreeSourceIndex(int* sourceIndex) {
    if (!available) return false;
    for (int i = 0; i < MaxNumSources; ++i) {
        ALint state;
        alGetSourcei(_sources[i], AL_SOURCE_STATE, &state);
        if(state != AL_PLAYING) {
            *sourceIndex = i;
            return true;
        }
    }
    return false;
}

bool SoundSystemAL::getBufferId(const SoundDesc& sound, ALuint* buf) {
    if (!available) return false;
    for (int i = 0; i < (int)_buffers.size(); ++i) {
        // Points to the same data buffer -> sounds equal
        if (_buffers[i].framePtr == sound.frames) {
            //LOGI("Found %p for %s!\n", sound.frames, sound.name.c_str());
            *buf = _buffers[i].bufferID;
            return true;
        }
    }

    if (!sound.isValid()) {
        LOGE("Err: sound is invalid @ getBufferId! %s\n", sound.name.c_str());
        return false;
    }
    
    ALuint bufferID;
    alGenBuffers(1, &bufferID);
    errIdString = "Gen buffer";
    checkError();
    
    ALenum format = (sound.byteWidth==2) ?
        (sound.channels==2? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16)
    :   (sound.channels==2? AL_FORMAT_STEREO8  : AL_FORMAT_MONO8);
    
    alBufferData(bufferID, format, sound.frames, sound.size, sound.frameRate);
    //LOGI("Creating %d (%p) from sound: '%s'\n", bufferID, sound.frames, sound.name.c_str());
    errIdString = "Buffer data";
    //LOGI("Creating buffer with data: %d (%d), %p, %d, %d\n", format, sound.byteWidth, sound.frames, sound.size, sound.frameRate);
    checkError();

    //LOGI("Sound ch: %d, fmt: %d, frames: %p, len: %f, fr: %d, sz: %d, numfr: %d\n", sound.channels, format, sound.frames, sound.length(), sound.frameRate, sound.size, sound.numFrames);
    
    
    Buffer buffer;
    buffer.inited = true;
    buffer.framePtr = sound.frames;
    buffer.bufferID = bufferID;
    *buf = bufferID;
    _buffers.push_back(buffer);

    // Embedded desktop PCM blobs live in static storage, so they must stay alive
    // for the whole process and must not be deleted here.
#if !defined(PLATFORM_DESKTOP)
    sound.destroy();
#endif
    return true;
}
