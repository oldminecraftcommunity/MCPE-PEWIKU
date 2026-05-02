#include "SoundEngine.h"
#include "../Options.h"
#include "../Minecraft.h"
#include "../../world/entity/Mob.h"
#include "../../platform/log.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#endif

#ifdef _WIN32
static bool _dirExists(const std::string& path) {
	const DWORD attr = GetFileAttributesA(path.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
}

static void _scanMusicRecursive(const std::string& root, std::vector<std::string>& out) {
	WIN32_FIND_DATAA findData;
	HANDLE handle = FindFirstFileA((root + "\\*").c_str(), &findData);
	if (handle == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		const char* name = findData.cFileName;
		if (!strcmp(name, ".") || !strcmp(name, "..")) {
			continue;
		}

		const std::string fullPath = root + "\\" + name;
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			_scanMusicRecursive(fullPath, out);
			continue;
		}

		const size_t dot = fullPath.find_last_of('.');
		if (dot == std::string::npos) {
			continue;
		}

		std::string ext = fullPath.substr(dot);
		for (size_t i = 0; i < ext.size(); ++i) {
			ext[i] = (char)tolower((unsigned char)ext[i]);
		}
		if (ext == ".wav") {
			out.push_back(fullPath);
		}
	} while (FindNextFileA(handle, &findData));

	FindClose(handle);
}
#endif


SoundEngine::SoundEngine( float maxDistance )
:	idCounter(0),
	mc(0),
	_x(0),
	_y(0),
	_z(0),
	_yRot(0),
	_invMaxDistance(1.0f / maxDistance)
#ifdef _WIN32
,	_musicOrderIndex(0),
	_musicWasEnabled(false),
	_musicHasOpenTrack(false),
	_loggedOpenALFallback(false)
#endif
{

}

SoundEngine::~SoundEngine()
{

}

void SoundEngine::init( Minecraft* mc, Options* options )
{
	this->mc = mc;
	this->options = options;

	if (/*!loaded && */(options == NULL || (options->sound != 0 || options->music != 0))) {
		loadLibrary();
	}

#if !defined(PRE_ANDROID23) && !defined(__APPLE__) && !defined(RPI)
	sounds.add("step.cloth", SA_cloth1);
	sounds.add("step.cloth", SA_cloth2);
	sounds.add("step.cloth", SA_cloth3);
	sounds.add("step.cloth", SA_cloth4);
	sounds.add("step.grass", SA_grass1);
	sounds.add("step.grass", SA_grass2);
	sounds.add("step.grass", SA_grass3);
	sounds.add("step.grass", SA_grass4);
	sounds.add("step.gravel", SA_gravel1);
	sounds.add("step.gravel", SA_gravel2);
	sounds.add("step.gravel", SA_gravel3);
	sounds.add("step.gravel", SA_gravel4);
	sounds.add("step.sand", SA_sand1);
	sounds.add("step.sand", SA_sand2);
	sounds.add("step.sand", SA_sand3);
	sounds.add("step.sand", SA_sand4);
	sounds.add("step.stone", SA_stone1);
	sounds.add("step.stone", SA_stone2);
	sounds.add("step.stone", SA_stone3);
	sounds.add("step.stone", SA_stone4);
	sounds.add("step.wood", SA_wood1);
	sounds.add("step.wood", SA_wood2);
	sounds.add("step.wood", SA_wood3);
	sounds.add("step.wood", SA_wood4);

	sounds.add("random.splash", SA_splash);
	sounds.add("random.explode", SA_explode);
	sounds.add("random.click", SA_click);
	sounds.add("random.break", SA_break);
	sounds.add("random.burp", SA_burp);
	sounds.add("fire.fire", SA_fire);
	sounds.add("fire.ignite", SA_ignite);

	sounds.add("random.door_open", SA_door_open);
	sounds.add("random.door_close", SA_door_close);
	sounds.add("random.chestclosed", SA_chestclosed);
	sounds.add("random.chestopen", SA_chestopen);
	sounds.add("random.pop", SA_pop);
	sounds.add("random.pop2", SA_pop2);
	sounds.add("random.hurt", SA_hurt);
	sounds.add("random.glass", SA_glass1);
	sounds.add("random.glass", SA_glass2);
	sounds.add("random.glass", SA_glass3);

	sounds.add("mob.sheep", SA_sheep1);
	sounds.add("mob.sheep", SA_sheep2);
	sounds.add("mob.sheep", SA_sheep3);
	sounds.add("mob.pig", SA_pig1);
	sounds.add("mob.pig", SA_pig2);
	sounds.add("mob.pig", SA_pig3);
	sounds.add("mob.pigdeath", SA_pigdeath);

	sounds.add("mob.cow", SA_cow1);
	sounds.add("mob.cow", SA_cow2);
	sounds.add("mob.cow", SA_cow3);
	sounds.add("mob.cow", SA_cow4);
	sounds.add("mob.cowhurt", SA_cowhurt1);
	sounds.add("mob.cowhurt", SA_cowhurt2);
	sounds.add("mob.cowhurt", SA_cowhurt3);

	sounds.add("mob.chicken", SA_chicken2);
	sounds.add("mob.chicken", SA_chicken3);
	sounds.add("mob.chickenhurt", SA_chickenhurt1);
	sounds.add("mob.chickenhurt", SA_chickenhurt2);

	sounds.add("mob.zombie", SA_zombie1);
	sounds.add("mob.zombie", SA_zombie2);
	sounds.add("mob.zombie", SA_zombie3); 
	sounds.add("mob.zombiedeath", SA_zombiedeath);
	sounds.add("mob.zombiehurt", SA_zombiehurt1);
	sounds.add("mob.zombiehurt", SA_zombiehurt2);

	sounds.add("mob.skeleton", SA_skeleton1);
	sounds.add("mob.skeleton", SA_skeleton2);
	sounds.add("mob.skeleton", SA_skeleton3);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt1);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt2);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt3);
	sounds.add("mob.skeletonhurt", SA_skeletonhurt4);
	sounds.add("mob.skeletondeath", SA_skeletondeath);

	sounds.add("mob.spider", SA_spider1);
	sounds.add("mob.spider", SA_spider2);
	sounds.add("mob.spider", SA_spider3);
	sounds.add("mob.spider", SA_spider4);
	sounds.add("mob.spiderdeath", SA_spiderdeath);

	sounds.add("mob.zombiepig.zpig", SA_zpig1);
	sounds.add("mob.zombiepig.zpig", SA_zpig2);
	sounds.add("mob.zombiepig.zpig", SA_zpig3);
	sounds.add("mob.zombiepig.zpig", SA_zpig4);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry1);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry2);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry3);
	sounds.add("mob.zombiepig.zpigangry", SA_zpigangry4);
	sounds.add("mob.zombiepig.zpigdeath", SA_zpigdeath);
	sounds.add("mob.zombiepig.zpighurt", SA_zpighurt1);
	sounds.add("mob.zombiepig.zpighurt", SA_zpighurt2);

	sounds.add("damage.fallbig", SA_fallbig1);
	sounds.add("damage.fallbig", SA_fallbig2);
	sounds.add("damage.fallsmall", SA_fallsmall);

	sounds.add("random.bow", SA_bow);
	sounds.add("random.bowhit", SA_bowhit1);
	sounds.add("random.bowhit", SA_bowhit2);
	sounds.add("random.bowhit", SA_bowhit3);
	sounds.add("random.bowhit", SA_bowhit4);

	sounds.add("mob.creeper", SA_creeper1);
	sounds.add("mob.creeper", SA_creeper2);
	sounds.add("mob.creeper", SA_creeper3);
	sounds.add("mob.creeper", SA_creeper4);
	sounds.add("mob.creeperdeath", SA_creeperdeath);
	sounds.add("random.eat", SA_eat1);
	sounds.add("random.eat", SA_eat2);
	sounds.add("random.eat", SA_eat3);
	sounds.add("random.fuse", SA_fuse);

#endif

#ifdef _WIN32
	discoverMusicTracks();
	_musicWasEnabled = false;
	if (!soundSystem.isAvailable() && !_loggedOpenALFallback) {
		_loggedOpenALFallback = true;
		LOGI("OpenAL library unavailable on Win32, audio playback is disabled...\n");
	}
#endif
}

void SoundEngine::enable( bool status )
{
#if defined(__APPLE__) || defined(_WIN32)
	soundSystem.enable(status);
#endif
}

void SoundEngine::updateOptions()
{
#ifdef _WIN32
	refreshMusicPlayback();
#endif
}

void SoundEngine::destroy()
{
#ifdef _WIN32
	stopMusicPlayback();
#endif
	//if (loaded) soundSystem.cleanup();
}

void SoundEngine::update( Mob* player, float a )
{
#ifdef _WIN32
	refreshMusicPlayback();
#endif
	if (/*!loaded || */options->sound == 0) return;
	if (player == NULL) return;

	_x = player->xo + (player->x - player->xo) * a;
	_y = player->yo + (player->y - player->yo) * a;
	_z = player->zo + (player->z - player->zo) * a;
	_yRot = player->yRotO + (player->yRot - player->yRotO) * a;

	soundSystem.setListenerAngle(_yRot);
	soundSystem.setListenerPos(_x, _y, _z); //@note: not used, since we translate all sounds to Player space
}

#ifdef _WIN32
void SoundEngine::discoverMusicTracks()
{
	_musicTracks.clear();
	_musicOrder.clear();
	_musicOrderIndex = 0;

	char modulePath[MAX_PATH] = { 0 };
	std::string baseDir = ".";
	const DWORD len = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
	if (len > 0 && len < MAX_PATH) {
		std::string exePath(modulePath, len);
		const size_t slash = exePath.find_last_of("\\/");
		if (slash != std::string::npos) {
			baseDir = exePath.substr(0, slash);
		}
	}

	std::vector<std::string> roots;
	roots.push_back(baseDir + "\\data\\music");
	roots.push_back(".\\data\\music");
	roots.push_back(".\\games\\com.mojang\\data\\music");
	roots.push_back("..\\..\\data\\music");

	for (size_t i = 0; i < roots.size(); ++i) {
		if (_dirExists(roots[i])) {
			_scanMusicRecursive(roots[i], _musicTracks);
		}
	}

	std::sort(_musicTracks.begin(), _musicTracks.end());
	_musicTracks.erase(std::unique(_musicTracks.begin(), _musicTracks.end()), _musicTracks.end());
}

void SoundEngine::stopMusicPlayback()
{
	soundSystem.stopMusic();
	_musicHasOpenTrack = false;
}

void SoundEngine::playNextMusicTrack()
{
	if (_musicTracks.empty()) {
		return;
	}

	if (_musicOrder.empty() || _musicOrderIndex >= (int)_musicOrder.size()) {
		_musicOrder.clear();
		_musicOrder.reserve(_musicTracks.size());
		for (int i = 0; i < (int)_musicTracks.size(); ++i) {
			_musicOrder.push_back(i);
		}

		for (int i = (int)_musicOrder.size() - 1; i > 0; --i) {
			const int j = random.nextInt(i + 1);
			std::swap(_musicOrder[i], _musicOrder[j]);
		}
		_musicOrderIndex = 0;
	}

	const std::string& path = _musicTracks[_musicOrder[_musicOrderIndex++]];
	stopMusicPlayback();
	_musicHasOpenTrack = soundSystem.playMusicWavFile(path, Mth::clamp(options->music, 0.0f, 1.0f));
}

void SoundEngine::refreshMusicPlayback()
{
	if (!options) {
		return;
	}

	const bool musicEnabled = options->music > 0.0f;
	if (!musicEnabled) {
		stopMusicPlayback();
		_musicWasEnabled = false;
		return;
	}

	if (!_musicWasEnabled) {
		_musicWasEnabled = true;
	}

	if (_musicTracks.empty()) {
		return;
	}

	if (!_musicHasOpenTrack) {
		playNextMusicTrack();
		return;
	}

	soundSystem.setMusicVolume(Mth::clamp(options->music, 0.0f, 1.0f));
	if (!soundSystem.isMusicPlaying()) {
		_musicHasOpenTrack = false;
		playNextMusicTrack();
	}
}
#endif

float SoundEngine::_getVolumeMult( float x, float y, float z )
{
	const float dx = x - _x;
	const float dy = y - _y;
	const float dz = z - _z;
	const float dist = Mth::sqrt(dx*dx + dy*dy + dz*dz);
	float fade = 1.0f - dist * _invMaxDistance;
	return (fade > 0.0f) ? (fade * fade) : 0.0f;
}

#if defined(PRE_ANDROID23)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	//volume *= (2.0f * _getVolumeMult(x, y, z))
	if ((volume *= options->sound) <= 0) return;

	volume *= _getVolumeMult(x, y, z);
	mc->platform()->playSound(name, volume, pitch);
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	//volume *= 2.0f;
	mc->platform()->playSound(name, volume, pitch);
}
#elif defined(__APPLE__)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, x-_x, y-_y, z-_z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (/*!loaded || */options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#elif defined(_WIN32)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume * _getVolumeMult(x, y, z), 0.0f, 1.0f);
	if (options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, x - _x, y - _y, z - _z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#elif defined(RPI)
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {}
#else
void SoundEngine::play(const std::string& name, float x, float y, float z, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume * _getVolumeMult(x, y, z), 0.0f, 1.0f);
	if (options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, x - _x, y - _y, z - _z, volume, pitch);
	}
}
void SoundEngine::playUI(const std::string& name, float volume, float pitch) {
	if ((volume *= options->sound) <= 0) return;

	volume = Mth::clamp(volume, 0.0f, 1.0f);
	if (/*!loaded || */options->sound == 0 || volume <= 0) return;

	SoundDesc sound;
	if (sounds.get(name, sound)) {
		soundSystem.playAt(sound, 0, 0, 0, volume, pitch);
	}
}
#endif
