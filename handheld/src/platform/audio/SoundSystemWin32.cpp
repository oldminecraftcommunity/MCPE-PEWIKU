#include "SoundSystemWin32.h"
#include "../../client/sound/Sound.h"

#ifdef _WIN32
#ifndef INITGUID
#define INITGUID
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <vector>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include <dsound.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "winmm.lib")

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

            if (formatTag != WAVE_FORMAT_PCM) {
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

SoundSystemWin32::SoundSystemWin32()
:   _device(NULL),
    _musicBuffer(NULL),
    _listenerX(0.0f),
    _listenerY(0.0f),
    _listenerZ(0.0f),
    _listenerDeg(0.0f)
{
    _voices.reserve(MAX_VOICES);
    initDevice();
}

SoundSystemWin32::~SoundSystemWin32()
{
    stopMusic();
    cleanupAllVoices();
    if (_device) {
        ((LPDIRECTSOUND8)_device)->Release();
        _device = NULL;
    }
}

void SoundSystemWin32::initDevice()
{
    if (_device) return;

    LPDIRECTSOUND8 ds = NULL;
    if (FAILED(DirectSoundCreate8(NULL, &ds, NULL))) {
        _device = NULL;
        return;
    }
    _device = ds;

    LPDIRECTSOUND8 device = (LPDIRECTSOUND8)_device;

    HWND wnd = GetForegroundWindow();
    if (!wnd) wnd = GetDesktopWindow();
    if (wnd) {
        device->SetCooperativeLevel(wnd, DSSCL_PRIORITY);
    }

    // Create a primary buffer once so DirectSound is fully initialized.
    DSBUFFERDESC desc;
    std::memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    LPDIRECTSOUNDBUFFER primary = NULL;
    if (SUCCEEDED(device->CreateSoundBuffer(&desc, &primary, NULL)) && primary) {
        WAVEFORMATEXTENSIBLE fmtx;
        std::memset(&fmtx, 0, sizeof(fmtx));
        fmtx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        fmtx.Format.nChannels = 2;
        fmtx.Format.nSamplesPerSec = 44100;
        fmtx.Format.wBitsPerSample = 16;
        fmtx.Format.nBlockAlign = 4;
        fmtx.Format.nAvgBytesPerSec = fmtx.Format.nSamplesPerSec * fmtx.Format.nBlockAlign;
        fmtx.Format.cbSize = 22;
        fmtx.Samples.wValidBitsPerSample = 16;
        fmtx.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
        fmtx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        primary->SetFormat((WAVEFORMATEX*)&fmtx);
        primary->Release();
    }
}

void SoundSystemWin32::cleanupStoppedVoices()
{
    LPDIRECTSOUNDBUFFER buffer = NULL;
    for (size_t i = 0; i < _voices.size();) {
        DWORD status = 0;
        buffer = (LPDIRECTSOUNDBUFFER)_voices[i].buffer;
        if (!buffer || FAILED(buffer->GetStatus(&status)) || !(status & DSBSTATUS_PLAYING)) {
            if (buffer) buffer->Release();
            _voices.erase(_voices.begin() + i);
            continue;
        }
        ++i;
    }
}

void SoundSystemWin32::cleanupAllVoices()
{
    for (size_t i = 0; i < _voices.size(); ++i) {
        LPDIRECTSOUNDBUFFER buffer = (LPDIRECTSOUNDBUFFER)_voices[i].buffer;
        if (buffer) {
            buffer->Stop();
            buffer->Release();
        }
    }
    _voices.clear();
}

long SoundSystemWin32::toDSVolume(float gain)
{
    if (gain <= 0.0001f) return DSBVOLUME_MIN;
    if (gain >= 1.0f) return DSBVOLUME_MAX;

    // 20*log10(gain) dB -> DirectSound hundredth-dB units
    float db = 20.0f * std::log10(gain);
    long out = (long)(db * 100.0f);
    if (out < DSBVOLUME_MIN) out = DSBVOLUME_MIN;
    if (out > DSBVOLUME_MAX) out = DSBVOLUME_MAX;
    return out;
}

long SoundSystemWin32::toDSPan(float pan)
{
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;
    return (long)(pan * 10000.0f);
}

void SoundSystemWin32::setListenerPos(float x, float y, float z)
{
    _listenerX = x;
    _listenerY = y;
    _listenerZ = z;
}

void SoundSystemWin32::setListenerAngle(float deg)
{
    _listenerDeg = deg;
}

bool SoundSystemWin32::playMusicWavFile(const std::string& path, float volume)
{
    if (!_device) return false;

    std::vector<char> pcm;
    int channels = 0;
    int bitsPerSample = 0;
    int sampleRate = 0;
    if (!_loadWavPcm(path, pcm, channels, bitsPerSample, sampleRate)) {
        return false;
    }

    stopMusic();

    WAVEFORMATEX fmt;
    std::memset(&fmt, 0, sizeof(fmt));
    fmt.wFormatTag = WAVE_FORMAT_PCM;
    fmt.nChannels = (WORD)channels;
    fmt.nSamplesPerSec = (DWORD)sampleRate;
    fmt.wBitsPerSample = (WORD)bitsPerSample;
    fmt.nBlockAlign = (WORD)((fmt.nChannels * fmt.wBitsPerSample) / 8);
    fmt.nAvgBytesPerSec = fmt.nSamplesPerSec * fmt.nBlockAlign;

    DSBUFFERDESC desc;
    std::memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
    desc.dwBufferBytes = (DWORD)pcm.size();
    desc.lpwfxFormat = &fmt;

    LPDIRECTSOUNDBUFFER buffer = NULL;
    if (FAILED(((LPDIRECTSOUND8)_device)->CreateSoundBuffer(&desc, &buffer, NULL)) || !buffer) {
        return false;
    }

    void* p1 = NULL;
    void* p2 = NULL;
    DWORD b1 = 0;
    DWORD b2 = 0;
    if (FAILED(buffer->Lock(0, (DWORD)pcm.size(), &p1, &b1, &p2, &b2, 0))) {
        buffer->Release();
        return false;
    }

    if (b1 > 0) std::memcpy(p1, &pcm[0], b1);
    if (p2 && b2 > 0) std::memcpy(p2, &pcm[0] + b1, b2);
    buffer->Unlock(p1, b1, p2, b2);

    buffer->SetVolume(toDSVolume(volume));
    buffer->SetCurrentPosition(0);
    if (FAILED(buffer->Play(0, 0, 0))) {
        buffer->Release();
        return false;
    }

    _musicBuffer = (void*)buffer;
    return true;
}

void SoundSystemWin32::stopMusic()
{
    LPDIRECTSOUNDBUFFER buffer = (LPDIRECTSOUNDBUFFER)_musicBuffer;
    if (!buffer) return;

    buffer->Stop();
    buffer->Release();
    _musicBuffer = NULL;
}

void SoundSystemWin32::setMusicVolume(float volume)
{
    LPDIRECTSOUNDBUFFER buffer = (LPDIRECTSOUNDBUFFER)_musicBuffer;
    if (!buffer) return;
    buffer->SetVolume(toDSVolume(volume));
}

bool SoundSystemWin32::isMusicPlaying() const
{
    LPDIRECTSOUNDBUFFER buffer = (LPDIRECTSOUNDBUFFER)_musicBuffer;
    if (!buffer) return false;

    DWORD status = 0;
    if (FAILED(buffer->GetStatus(&status))) return false;
    return (status & DSBSTATUS_PLAYING) != 0;
}

void SoundSystemWin32::playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch)
{
    (void)y;
    if (!_device) return;
    LPDIRECTSOUND8 device = (LPDIRECTSOUND8)_device;
    if (!sound.isValid() || !sound.frames || sound.size <= 0) return;
    if (sound.channels <= 0 || sound.byteWidth <= 0 || sound.frameRate <= 0) return;
    if (sound.channels > 2) return;
    if (sound.byteWidth != 1 && sound.byteWidth != 2) return;
    if (volume <= 0.0f) return;

    cleanupStoppedVoices();
    if ((int)_voices.size() >= MAX_VOICES) return;

    const float dx = x;
    const float dz = z;
    const float dist = std::sqrt(dx * dx + dz * dz);

    float pan = 0.0f;
    if (dist > 0.001f) {
        const float rad = _listenerDeg * 0.01745329251994329577f;
        const float rightX = std::cos(rad);
        const float rightZ = -std::sin(rad);
        pan = (dx * rightX + dz * rightZ) / dist;
    }

    float gain = volume;
    if (gain < 0.0f) gain = 0.0f;
    if (gain > 1.0f) gain = 1.0f;

    // Convert to mono first, then apply software stereo pan.
    const int srcFrameBytes = sound.channels * sound.byteWidth;
    const int numFrames = sound.size / srcFrameBytes;
    const int outFrameBytes = 4; // stereo 16-bit
    const int outSize = numFrames * outFrameBytes;
    if (numFrames <= 0 || outSize <= 0) return;

    char* mixed = new char[outSize];
    short* outSamples = (short*)mixed;

    // direct equal-power pan so left,right placement remains obvious,not true 3D
    float p = pan;
    if (p < -1.0f) p = -1.0f;
    if (p > 1.0f) p = 1.0f;

    const float theta = (p + 1.0f) * 0.78539816339f; // [0..pi/2]
    float panL = std::cos(theta);
    float panR = std::sin(theta);

    for (int i = 0; i < numFrames; ++i) {
        float mono = 0.0f;
        if (sound.byteWidth == 1) {
            const unsigned char* s = (const unsigned char*)(sound.frames + i * srcFrameBytes);
            if (sound.channels == 2) {
                float l = ((int)s[0] - 128) / 128.0f;
                float r = ((int)s[1] - 128) / 128.0f;
                mono = 0.5f * (l + r);
            } else {
                mono = ((int)s[0] - 128) / 128.0f;
            }
        } else {
            const short* s = (const short*)(sound.frames + i * srcFrameBytes);
            if (sound.channels == 2) {
                float l = s[0] / 32768.0f;
                float r = s[1] / 32768.0f;
                mono = 0.5f * (l + r);
            } else {
                mono = s[0] / 32768.0f;
            }
        }

        if (mono < -1.0f) mono = -1.0f;
        if (mono > 1.0f) mono = 1.0f;
        float l = mono * gain * panL;
        float r = mono * gain * panR;
        if (l < -1.0f) l = -1.0f;
        if (l > 1.0f) l = 1.0f;
        if (r < -1.0f) r = -1.0f;
        if (r > 1.0f) r = 1.0f;
        outSamples[i * 2 + 0] = (short)(l * 32767.0f);
        outSamples[i * 2 + 1] = (short)(r * 32767.0f);
    }

    WAVEFORMATEXTENSIBLE fmtx;
    std::memset(&fmtx, 0, sizeof(fmtx));
    fmtx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    fmtx.Format.nChannels = 2;
    fmtx.Format.nSamplesPerSec = (DWORD)sound.frameRate;
    fmtx.Format.wBitsPerSample = 16;
    fmtx.Format.nBlockAlign = 4;
    fmtx.Format.nAvgBytesPerSec = fmtx.Format.nSamplesPerSec * fmtx.Format.nBlockAlign;
    fmtx.Format.cbSize = 22;
    fmtx.Samples.wValidBitsPerSample = 16;
    fmtx.dwChannelMask = SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT;
    fmtx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

    DSBUFFERDESC desc;
    std::memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS;
    desc.dwBufferBytes = (DWORD)outSize;
    desc.lpwfxFormat = (WAVEFORMATEX*)&fmtx;

    LPDIRECTSOUNDBUFFER buffer = NULL;
    if (FAILED(device->CreateSoundBuffer(&desc, &buffer, NULL)) || !buffer) {
        delete[] mixed;
        return;
    }

    void* p1 = NULL;
    void* p2 = NULL;
    DWORD b1 = 0;
    DWORD b2 = 0;
    if (FAILED(buffer->Lock(0, (DWORD)outSize, &p1, &b1, &p2, &b2, 0))) {
        buffer->Release();
        delete[] mixed;
        return;
    }

    std::memcpy(p1, mixed, b1);
    if (p2 && b2) std::memcpy(p2, mixed + b1, b2);
    buffer->Unlock(p1, b1, p2, b2);
    delete[] mixed;

    buffer->SetVolume(DSBVOLUME_MAX);

    float adjustedPitch = pitch;
    if (adjustedPitch < 0.25f) adjustedPitch = 0.25f;
    if (adjustedPitch > 4.0f) adjustedPitch = 4.0f;

    DWORD freq = (DWORD)(sound.frameRate * adjustedPitch);
    if (freq < DSBFREQUENCY_MIN) freq = DSBFREQUENCY_MIN;
    if (freq > DSBFREQUENCY_MAX) freq = DSBFREQUENCY_MAX;
    buffer->SetFrequency(freq);

    buffer->SetCurrentPosition(0);
    if (FAILED(buffer->Play(0, 0, 0))) {
        buffer->Release();
        return;
    }

    ActiveVoice voice;
    voice.buffer = (void*)buffer;
    _voices.push_back(voice);
}

#else
SoundSystemWin32::SoundSystemWin32() {}
SoundSystemWin32::~SoundSystemWin32() {}
bool SoundSystemWin32::playMusicWavFile(const std::string& path, float volume)
{
    (void)path;
    (void)volume;
    return false;
}
void SoundSystemWin32::stopMusic() {}
void SoundSystemWin32::setMusicVolume(float volume)
{
    (void)volume;
}
bool SoundSystemWin32::isMusicPlaying() const
{
    return false;
}
void SoundSystemWin32::setListenerPos(float x, float y, float z)
{
    (void)x;
    (void)y;
    (void)z;
}
void SoundSystemWin32::setListenerAngle(float deg)
{
    (void)deg;
}
void SoundSystemWin32::playAt(const SoundDesc& desc, float x, float y, float z, float volume, float pitch)
{
    (void)desc;
    (void)x;
    (void)y;
    (void)z;
    (void)volume;
    (void)pitch;
}
#endif
