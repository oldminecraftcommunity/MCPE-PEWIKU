#include "Common.h"
#include "../SharedConstants.h"
#include <sstream>
#include <ctime>
#ifdef WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/time.h>
#endif

namespace Common {
	std::string getGameVersionString() {
		return "v" + getGameVersionStringNet() + " alpha";
	}

	std::string getGameVersionStringNet() {
		std::stringstream ss;
		ss << SharedConstants::MajorVersion << "." << SharedConstants::MinorVersion << "." << SharedConstants::PatchVersion;
		if (SharedConstants::RevisionVersion > 0) {
			ss << "." << SharedConstants::RevisionVersion;
		}
		return ss.str();
	}

	int getRawTimeS() {
#ifdef WIN32
		return (int)time(NULL);
#else
		struct timeval _time;
		gettimeofday(&_time, 0);
		return _time.tv_sec;
#endif
	}

	long long getRawTimeMs() {
#ifdef WIN32
		// `time()` only updates once per second on Windows, which makes
		// guiTime jump in large steps and visibly stutter menu animations.
		return (long long)GetTickCount64();
#else
		struct timeval _time;
		gettimeofday(&_time, 0);
		return (long long)_time.tv_sec * 1000 + _time.tv_usec / 1000;
#endif
	}
}
