#include "Options.h"
#include "OptionStrings.h"
#include "Minecraft.h"
#include "../platform/log.h"
#include "../util/Mth.h"
#include "../world/Difficulty.h"
#include <cmath>
#include <sstream>
/*static*/
bool Options::debugGl = false;

void Options::init(Minecraft* mc, const std::string& workingDirectory) {
	minecraft = mc;
	optionsFile.setSettingsFolderPath(workingDirectory);
	initDefaultValues();
}

void Options::initDefaultValues() {
	beta = 0;
	patch = 0;
	minor = 0;
	major = 0;
	difficulty = Difficulty::NORMAL;
	hideGui = false;
	thirdPersonView = false;
	renderDebug = false;

	isFlying = false; // by F in debug
	fixedCamera = false; // render
	flySpeed = 1;
	cameraSpeed = 1;
	useMouseForDigging = false;
	
	destroyVibration = true;
	fancySkies = true;
	animateTextures = true;
	guiScale = 0;

	isLeftHanded = false;

	isJoyTouchArea = false;

	music = 1.0f;
	sound = 1.0f;
	sensitivity = 0.5f;
	invertYMouse = false;
	viewDistance = 2;
	bobView = true;
	anaglyph3d = false;
	limitFramerate = false;
	fancyGraphics = true;
	brightness = 1.0f;
	ambientOcclusion = true;
	fov = 70;
	isSplitControls = 0;
	useTouchScreen = !minecraft->supportNonTouchScreen();
	
	float pmm = minecraft->platform()->getPixelsPerMillimeter();
	if (pmm > 12.0f) pmm = 12.0f;
	else if (pmm <= 3.0f) pmm = 3.0f;
	pixelsPerMillimeter = pmm;

	//skin     = "Default";
	username = "Steve";
	serverVisible = true;

	// comment = in 0.8.1
	keyUp	= KeyMapping("key.forward", Keyboard::KEY_W); // 87
	keyLeft		= KeyMapping("key.left", Keyboard::KEY_A); // 65
	keyDown		= KeyMapping("key.back", Keyboard::KEY_S); // 83
	keyRight	= KeyMapping("key.right", Keyboard::KEY_D); // 67
	keyJump		= KeyMapping("key.jump", Keyboard::KEY_SPACE); // 32
	keyInventory	= KeyMapping("key.inventory", Keyboard::KEY_E); // 69
	keySneak	= KeyMapping("key.sneak", Keyboard::KEY_LSHIFT); // 10
	keyCraft	= KeyMapping("key.crafting", Keyboard::KEY_P); // 81
	keyDrop		= KeyMapping("key.drop", Keyboard::KEY_Q); // 81
	keyChat		= KeyMapping("key.chat", Keyboard::KEY_T); // 84
	keyFog		= KeyMapping("key.fog", Keyboard::KEY_F); // 70
	keyDestroy	= KeyMapping("key.destroy", 88); // @todo @fix
	keyUse		= KeyMapping("key.use", Keyboard::KEY_U); // 85
	
	//const int Unused = 99999;
	keyMenuNext     = KeyMapping("key.menu.next",     40);
	keyMenuPrevious = KeyMapping("key.menu.previous", 38);
	keyMenuOk       = KeyMapping("key.menu.ok",       13);
	keyMenuCancel   = KeyMapping("key.menu.cancel",   8);

	int k = 0;
	keyMappings[k++] = &keyUp;
	keyMappings[k++] = &keyLeft;
	keyMappings[k++] = &keyDown;
	keyMappings[k++] = &keyRight;
	keyMappings[k++] = &keyJump;
	keyMappings[k++] = &keySneak;
	keyMappings[k++] = &keyDrop;
	keyMappings[k++] = &keyInventory;
	keyMappings[k++] = &keyChat;
	keyMappings[k++] = &keyFog;
	keyMappings[k++] = &keyDestroy;
	keyMappings[k++] = &keyUse;

	keyMappings[k++] = &keyMenuNext;
	keyMappings[k++] = &keyMenuPrevious;
	keyMappings[k++] = &keyMenuOk;
	keyMappings[k++] = &keyMenuCancel;

//	"Polymorphism" at it's worst. At least it's better to have it here
//	for now, then to have it spread all around the game code (even if
//	it would be slightly better performance with it inlined. Should
//  probably create separate subclasses (or read from file).

#if !defined(PLATFORM_DESKTOP)
	keyUp.key		= 19;
	keyDown.key		= 20;
	keyLeft.key		= 21;
	keyRight.key		= 22;
	keyJump.key		= 23;
	keyUse.key		= 103;
	keyDestroy.key		= 102;
	keyCraft.key 	= 109;

	keyMenuNext.key		= 20;
	keyMenuPrevious.key	= 19;
	keyMenuOk.key		= 23;
	keyMenuCancel.key	= 4;
#endif

	setAdditionalHiddenOptions({});
}

std::vector<int> Options::DIFFICULTY_LEVELS = 
	{ Difficulty::PEACEFUL, Difficulty::PEACEFUL };
std::vector<int> Options::RENDER_DISTANCE_LEVELS = {3, 2, 1, 0};

// (ID, Option Name /i18n key/, IsFloat, IsBoolean)
const Options::Option
	Options::Option::MUSIC				 (0, "options.music",		true, false),
	Options::Option::SOUND				 (1, "options.sound",		true, false),
	Options::Option::INVERT_MOUSE		 (2, "options.invertMouse",	false, true),
	Options::Option::SENSITIVITY		 (3, "options.sensitivity",	true, false),
	Options::Option::RENDER_DISTANCE	 (4, "options.renderDistance",false, false),
	Options::Option::VIEW_BOBBING		 (5, "options.viewBobbing",	false, true),
	Options::Option::ANAGLYPH			 (6, "options.anaglyph",		false, true),
	Options::Option::LIMIT_FRAMERATE	 (7, "options.limitFramerate",false, true),
	Options::Option::DIFFICULTY			 (8, "options.difficulty",	false, false),
	Options::Option::GRAPHICS			 (9, "options.graphics",		false, true),
	Options::Option::AMBIENT_OCCLUSION	 (10, "options.ao",		false, true),
	Options::Option::GUI_SCALE			 (11, "options.guiScale",	false, false),
	Options::Option::THIRD_PERSON		 (12, "options.thirdperson",	false, true),
    Options::Option::HIDE_GUI			 (13, "options.hidegui",     false, true),
	Options::Option::SERVER_VISIBLE		 (14, "options.servervisible", false, true),
	Options::Option::LEFT_HANDED		 (15, "options.lefthanded", false, true),
	Options::Option::USE_TOUCHSCREEN	 (16, "options.usetouchscreen", false, true),
	Options::Option::USE_TOUCH_JOYPAD	 (17, "options.usetouchpad", false, true),
	Options::Option::DESTROY_VIBRATION   (18, "options.destroyvibration", false, true),
	Options::Option::FANCY_SKIES		 (19, "options.fancyskies", false, true),
	Options::Option::ANIMATE_TEXTURES	 (20, "options.animatetextures", false, true),
	Options::Option::PIXELS_PER_MILLIMETER(21, "options.pixelspermilimeter", true, false),
	Options::Option::NAME				 (22, "options.name", false, false),
	Options::Option::FOV				 (23, "options.fov", true, false);

/* private */
const float Options::SOUND_MIN_VALUE = 0.0f;
const float Options::SOUND_MAX_VALUE = 1.0f;
const float Options::MUSIC_MIN_VALUE = 0.0f;
const float Options::MUSIC_MAX_VALUE = 1.0f;
const float Options::SENSITIVITY_MIN_VALUE = 0.0f;
const float Options::SENSITIVITY_MAX_VALUE = 1.0f;
const float Options::PIXELS_PER_MILLIMETER_MIN_VALUE = 3.0f;
const float Options::PIXELS_PER_MILLIMETER_MAX_VALUE = 4.0f;
const float Options::FOV_MIN_VALUE = 30.0f;
const float Options::FOV_MAX_VALUE = 110.0f;
const int DIFFICULY_LEVELS[] = {
	Difficulty::PEACEFUL,
	Difficulty::NORMAL
};

/*private*/
const char* Options::RENDER_DISTANCE_NAMES[] = {
	"options.renderDistance.far",
	"options.renderDistance.normal",
	"options.renderDistance.short",
	"options.renderDistance.tiny"
};

/*private*/
const char* Options::DIFFICULTY_NAMES[] = {
	"options.difficulty.peaceful",
	"options.difficulty.easy",
	"options.difficulty.normal",
	"options.difficulty.hard"
};

/*private*/
const char* Options::GUI_SCALE[] = {
	"options.guiScale.auto",
	"options.guiScale.small",
	"options.guiScale.normal",
	"options.guiScale.large"
};

void Options::update()
{
	StringVector optionStrings = optionsFile.getOptionStrings();
	for (unsigned int i = 0; i < optionStrings.size(); i += 2) {
		const std::string& key = optionStrings[i];
		const std::string& value = optionStrings[i+1];

        	//LOGI("reading key: %s (%s)\n", key.c_str(), value.c_str());

		if (key == OptionStrings::Multiplayer_Username) {
			if (value.size()) {
				username = value;
			} else {
				username = "Steve";
			}
		} else if (key == OptionStrings::Multiplayer_ServerVisible) {
			readBool(value, serverVisible);
		} else if (key == OptionStrings::Controls_Sensitivity) {
			float val;
			if (readFloat(value, val)) {
				sensitivity = val;
			}
		} else if (key == OptionStrings::Controls_InvertMouse) {
			readBool(value, invertYMouse);
		} else if (key == OptionStrings::Controls_IsLefthanded) {
			readBool(value, isLeftHanded);
		} else if (key == OptionStrings::Controls_UseTouchJoypad) {
			readBool(value, isJoyTouchArea);
			if (!minecraft->useTouchscreen())
				isJoyTouchArea = false;
		} else if (key == OptionStrings::Controls_FeedbackVibration) {
			readBool(value, destroyVibration);
		} else if (key == OptionStrings::Graphics_RenderDistance) {
			readInt(value, viewDistance);
		} else if (key == OptionStrings::Graphics_PixelsPerMilimeter) {
			readFloat(value, pixelsPerMillimeter);
			if (pixelsPerMillimeter > 12.0f) pixelsPerMillimeter = 12.0f;
			else if (pixelsPerMillimeter <= 3.0f) pixelsPerMillimeter = 3.0f;
		} else if (key == OptionStrings::Graphics_FancyGraphics) {
			readBool(value, fancyGraphics);
		} else if (key == OptionStrings::Graphics_FancySkies) {
			readBool(value, fancySkies);
		} else if (key == OptionStrings::Graphics_AnimateTextures) {
			readBool(value, animateTextures);
		} else if (key == OptionStrings::Game_ThirdPerson) {
			readBool(value, thirdPersonView);
		} else if (key == OptionStrings::Controls_UseTouchScreen) {
			if (minecraft->supportNonTouchScreen()) {
				readBool(value, useTouchScreen);
			} else {
				useTouchScreen = true;
			}
		} else if (key == OptionStrings::Graphics_HideGUI) {
			readBool(value, hideGui);
		} else if (key == OptionStrings::Audio_Sound) {
			readFloat(value, sound);
		} else if (key == OptionStrings::Game_DifficultyLevel) {
			readInt(value, difficulty);
			if (difficulty != 0 && difficulty != 2)
				difficulty = 2;
		} else if (key == OptionStrings::Last_Game_Version_Major) {
			readInt(value, major);
		} else if (key == OptionStrings::Last_Game_Version_Minor) {
			readInt(value, minor);
		} else if (key == OptionStrings::Last_Game_Version_Patch) {
			readInt(value, patch);
		} else if (key == OptionStrings::Last_Game_Version_Beta) {
			readInt(value, beta);
		}
	}
}

void Options::validateVersion() {
	if (major != 0 || minor != 8 || patch != 1 || beta != 0) {
		minecraft->onUpdatedClient(major, minor, patch, beta);
		major = 0;
		minor = 8;
		patch = 1;
		beta = 0;
		save();
	}
}

void Options::load()
{
	StringVector strings = optionsFile.getOptionStrings();
	for (size_t i = 0; i + 1 < strings.size(); i += 2) {
		std::string key = strings[i];
		std::string value = strings[i + 1];

		if (key == OptionStrings::Multiplayer_Username) username = value;
		else if (key == OptionStrings::Game_DifficultyLevel) readInt(value, difficulty);
		else if (key == OptionStrings::Game_ThirdPerson) readBool(value, thirdPersonView);
		else if (key == OptionStrings::Graphics_Fov) readInt(value, fov);
		else if (key == OptionStrings::Graphics_PixelsPerMilimeter) readFloat(value, pixelsPerMillimeter);
		else if (key == OptionStrings::Multiplayer_ServerVisible) readBool(value, serverVisible);
		else if (key == OptionStrings::Controls_Sensitivity) readFloat(value, sensitivity);
		else if (key == OptionStrings::Controls_InvertMouse) readBool(value, invertYMouse);
		else if (key == OptionStrings::Controls_IsLefthanded) readBool(value, isLeftHanded);
		else if (key == OptionStrings::Controls_UseTouchScreen) readBool(value, useTouchScreen);
		else if (key == OptionStrings::Controls_UseTouchJoypad) readBool(value, isJoyTouchArea);
		else if (key == OptionStrings::Controls_FeedbackVibration) readBool(value, destroyVibration);
		else if (key == OptionStrings::Graphics_RenderDistance) readInt(value, viewDistance);
		else if (key == OptionStrings::Graphics_FancyGraphics) readBool(value, fancyGraphics);
		else if (key == OptionStrings::Graphics_FancySkies) readBool(value, fancySkies);
		else if (key == OptionStrings::Graphics_AnimateTextures) readBool(value, animateTextures);
		else if (key == OptionStrings::Graphics_HideGUI) readBool(value, hideGui);
		else if (key == OptionStrings::Audio_Sound) readFloat(value, sound);
		else if (key == OptionStrings::Audio_Music) readFloat(value, music);
		else if (key == OptionStrings::Last_Game_Version_Major) readInt(value, major);
		else if (key == OptionStrings::Last_Game_Version_Minor) readInt(value, minor);
		else if (key == OptionStrings::Last_Game_Version_Patch) readInt(value, patch);
		else if (key == OptionStrings::Last_Game_Version_Beta) readInt(value, beta);
	}
}

void Options::save()
{
	StringVector stringVec;
	// Audio
	addOptionToSaveOutput(stringVec, OptionStrings::Audio_Music, music);
	addOptionToSaveOutput(stringVec, OptionStrings::Audio_Sound, sound);

	// Game
	addOptionToSaveOutput(stringVec, OptionStrings::Multiplayer_Username, username);
	addOptionToSaveOutput(stringVec, OptionStrings::Multiplayer_ServerVisible, serverVisible);
	addOptionToSaveOutput(stringVec, OptionStrings::Game_DifficultyLevel, difficulty);
	addOptionToSaveOutput(stringVec, OptionStrings::Game_ThirdPerson, thirdPersonView);

	// Graphics
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_FancyGraphics, fancyGraphics);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_Fov, fov);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_GuiScale, guiScale);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_PixelsPerMilimeter, pixelsPerMillimeter);

	// Input
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_InvertMouse, invertYMouse);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_Sensitivity, sensitivity);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_IsLefthanded, isLeftHanded);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_UseTouchScreen, useTouchScreen);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_UseTouchJoypad, isJoyTouchArea);
	addOptionToSaveOutput(stringVec, OptionStrings::Controls_FeedbackVibration, destroyVibration);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_RenderDistance, viewDistance);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_FancySkies, fancySkies);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_AnimateTextures, animateTextures);
	addOptionToSaveOutput(stringVec, OptionStrings::Graphics_HideGUI, hideGui);
	addOptionToSaveOutput(stringVec, OptionStrings::Last_Game_Version_Major, major);
	addOptionToSaveOutput(stringVec, OptionStrings::Last_Game_Version_Minor, minor);
	addOptionToSaveOutput(stringVec, OptionStrings::Last_Game_Version_Patch, patch);
	addOptionToSaveOutput(stringVec, OptionStrings::Last_Game_Version_Beta, beta);

	optionsFile.save(stringVec);
}

void Options::toggle(const Option* option, int dir) {
	if (option == &Option::INVERT_MOUSE) {
		invertYMouse = !invertYMouse;
	} else if (option == &Option::RENDER_DISTANCE) {
		viewDistance = (viewDistance + dir) & 3;
	} else if (option == &Option::GUI_SCALE) {
		guiScale = (guiScale + dir) & 3;
	} else if (option == &Option::VIEW_BOBBING) {
		bobView = !bobView;
	} else if (option == &Option::THIRD_PERSON) {
		thirdPersonView = !thirdPersonView;
	} else if (option == &Option::HIDE_GUI) {
		hideGui = !hideGui;
	} else if (option == &Option::SERVER_VISIBLE) {
		serverVisible = !serverVisible;
	} else if (option == &Option::LEFT_HANDED) {
		isLeftHanded = !isLeftHanded;
	} else if (option == &Option::USE_TOUCHSCREEN) {
		useTouchScreen = !useTouchScreen;
	} else if (option == &Option::USE_TOUCH_JOYPAD) {
		isJoyTouchArea = !isJoyTouchArea;
	} else if (option == &Option::DESTROY_VIBRATION) {
		destroyVibration = !destroyVibration;
	} else if (option == &Option::GRAPHICS) {
		fancyGraphics = !fancyGraphics;
	} else if (option == &Option::FANCY_SKIES) {
		fancySkies = !fancySkies;
	} else if (option == &Option::ANIMATE_TEXTURES) {
		animateTextures = !animateTextures;
	} else if (option == &Option::LIMIT_FRAMERATE) {
		limitFramerate = !limitFramerate;
	} else if (option == &Option::DIFFICULTY) {
		difficulty = (difficulty + dir) & 3;
	}
	
	notifyOptionUpdate(option, getBooleanValue(option));
	save();
}

void Options::set(const Option* item, float value) {
	if (item == &Option::MUSIC) {
		music = value;
	} else if (item == &Option::SOUND) {
		sound = value;
	} else if (item == &Option::SENSITIVITY) {
		sensitivity = value;
	} else if (item == &Option::FOV) {
		fov = Mth::clamp(Mth::floor(value + 0.5f), (int)FOV_MIN_VALUE, (int)FOV_MAX_VALUE);
	} else if (item == &Option::PIXELS_PER_MILLIMETER) {
		pixelsPerMillimeter = value;
	}
	notifyOptionUpdate(item, value);
	save();
}

void Options::set(const Option* item, int value) {
	if (item == &Option::DIFFICULTY) {
		difficulty = value;
	} else if (item == &Option::RENDER_DISTANCE) {
		viewDistance = value;
	}
	notifyOptionUpdate(item, value);
	save();
}

void Options::set(const Option* item, const std::string& value) {
	if (item == &Option::NAME) {
		username = value.empty() ? "Steve" : value;
		// if (minecraft->user) minecraft->user->username = username;
	}
}

int Options::getIntValue(const Option* item) {
	if (item == &Option::DIFFICULTY)
		return difficulty;
	if (item == &Option::RENDER_DISTANCE)
		return viewDistance;
	if (item == &Option::GUI_SCALE)
		return guiScale;
	return 0;
}

float Options::getProgressValue(const Option* item) {
	if (item == &Option::MUSIC)
		return music;
	if (item == &Option::SOUND)
		return sound;
	if (item == &Option::SENSITIVITY)
		return sensitivity;
	if (item == &Option::FOV)
		return float(fov);
	if (item == &Option::PIXELS_PER_MILLIMETER)
		return pixelsPerMillimeter;
	return 0.0f;
}

bool Options::getBooleanValue(const Option* item) {
	if (item == &Option::INVERT_MOUSE)
		return invertYMouse;
	if (item == &Option::VIEW_BOBBING)
		return bobView;
	if (item == &Option::LIMIT_FRAMERATE)
		return limitFramerate;
	if (item == &Option::THIRD_PERSON)
		return thirdPersonView;
	if (item == &Option::HIDE_GUI)
		return hideGui;
	if (item == &Option::SERVER_VISIBLE)
		return serverVisible;
	if (item == &Option::LEFT_HANDED)
		return isLeftHanded;
	if (item == &Option::USE_TOUCHSCREEN)
		return useTouchScreen;
	if (item == &Option::USE_TOUCH_JOYPAD)
		return isJoyTouchArea;
	if (item == &Option::DESTROY_VIBRATION)
		return destroyVibration;
	if (item == &Option::FANCY_SKIES)
		return fancySkies;
	if (item == &Option::ANIMATE_TEXTURES)
		return animateTextures;
	if (item == &Option::GRAPHICS)
		return fancyGraphics;
	return false;
}

float Options::getProgrssMin(const Option* item) {
	if (item == &Option::MUSIC || item == &Option::SOUND || item == &Option::SENSITIVITY)
		return 0.0f;
	if (item == &Option::FOV)
		return FOV_MIN_VALUE;
	if (item == &Option::PIXELS_PER_MILLIMETER)
		return 3.0f;
	return 0.0f;
}

float Options::getProgrssMax(const Option* item) {
	if (item == &Option::MUSIC || item == &Option::SOUND || item == &Option::SENSITIVITY)
		return 1.0f;
	if (item == &Option::FOV)
		return FOV_MAX_VALUE;
	if (item == &Option::PIXELS_PER_MILLIMETER)
		return 12.0f;
	return 1.0f;
}

std::string Options::getKeyDescription(int i) {
	return "Options::getKeyDescription not implemented";
}

std::string Options::getKeyMessage(int i) {
	return "Options::getKeyMessage not implemented";
}

void Options::setKey(int i, int key) {
	keyMappings[i]->key = key;
	save();
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, bool boolValue) {
	std::stringstream ss;
	ss << name << ":" << boolValue;
	stringVector.push_back(ss.str());
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, float floatValue) {
	std::stringstream ss;
	ss << name << ":" << floatValue;
	stringVector.push_back(ss.str());
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, int intValue) {
	std::stringstream ss;
	ss << name << ":" << intValue;
	stringVector.push_back(ss.str());
}
void Options::addOptionToSaveOutput(StringVector& stringVector, std::string name, const std::string& stringValue) {
	std::stringstream ss;
	ss << name << ":" << stringValue;
	stringVector.push_back(ss.str());
}

std::string Options::getMessage( const Option* item )
{
	return "Options::getMessage - Not implemented";

	//Language language = Language.getInstance();
	//std::string caption = language.getElement(item.getCaptionId()) + ": ";

	//if (item.isProgress()) {
	//    float progressValue = getProgressValue(item);

	//    if (item == Option.SENSITIVITY) {
	//        if (progressValue == 0) {
	//            return caption + language.getElement("options.sensitivity.min");
	//        }
	//        if (progressValue == 1) {
	//            return caption + language.getElement("options.sensitivity.max");
	//        }
	//        return caption + (int) (progressValue * 200) + "%";
	//    } else {
	//        if (progressValue == 0) {
	//            return caption + language.getElement("options.off");
	//        }
	//        return caption + (int) (progressValue * 100) + "%";
	//    }
	//} else if (item.isBoolean()) {

	//    bool booleanValue = getBooleanValue(item);
	//    if (booleanValue) {
	//        return caption + language.getElement("options.on");
	//    }
	//    return caption + language.getElement("options.off");
	//} else if (item == Option.RENDER_DISTANCE) {
	//    return caption + language.getElement(RENDER_DISTANCE_NAMES[viewDistance]);
	//} else if (item == Option.DIFFICULTY) {
	//    return caption + language.getElement(DIFFICULTY_NAMES[difficulty]);
	//} else if (item == Option.GUI_SCALE) {
	//    return caption + language.getElement(GUI_SCALE[guiScale]);
	//} else if (item == Option.GRAPHICS) {
	//    if (fancyGraphics) {
	//        return caption + language.getElement("options.graphics.fancy");
	//    }
	//    return caption + language.getElement("options.graphics.fast");
	//}

	//return caption;
}

/*static*/
bool Options::readFloat(const std::string& string, float& value) {
	if (string == "true" || string == "YES")  { value = 1; return true; }
	if (string == "false" || string == "NO") { value = 0; return true; }
#ifdef _WIN32
	if (sscanf_s(string.c_str(), "%f", &value))
		return true;
#else
	if (sscanf(string.c_str(), "%f", &value))
		return true;
#endif
	return false;
}

/*static*/
bool Options::readInt(const std::string& string, int& value) {
	if (string == "true" || string == "YES")  { value = 1; return true; }
	if (string == "false" || string == "NO") { value = 0; return true; }
#ifdef _WIN32
	if (sscanf_s(string.c_str(), "%d", &value))
		return true;
#else
	if (sscanf(string.c_str(), "%d", &value))
		return true;
#endif
	return false;
}

/*static*/
bool Options::readBool(const std::string& string, bool& value) {
	std::string s = Util::stringTrim(string);
	if (string == "true" || string == "1" || string == "YES")  { value = true;  return true; }
	if (string == "false" || string == "0" || string == "NO") { value = false; return true; }
	return false;
}

void Options::notifyOptionUpdate( const Option* option, bool value ) {
	minecraft->optionUpdated(option, value);
}

void Options::notifyOptionUpdate( const Option* option, float value ) {
	minecraft->optionUpdated(option, value);
}

void Options::notifyOptionUpdate( const Option* option, int value ) {
	minecraft->optionUpdated(option, value);
}

void Options::setAdditionalHiddenOptions(const std::vector<const Option*>& options) {
	hiddenOptions = options;
}

bool Options::hideOption(const Option* option) {
	if (option == &Option::USE_TOUCHSCREEN && !minecraft->supportNonTouchScreen()) {
		return true;
	}
	for (auto hidden : hiddenOptions) {
		if (hidden == option) return true;
	}
	return false;
}

bool Options::canModify(const Option* option) {
	// 0.8 logic: return option != &NAME || mojangConnector->getStatus() == 0
	return true; // Simplified for now
}

std::vector<int> Options::getValues(const Option* option) {
	if (option == &Option::DIFFICULTY) return DIFFICULTY_LEVELS;
	if (option == &Option::RENDER_DISTANCE) return RENDER_DISTANCE_LEVELS;
	return {};
}

std::string Options::getStringValue(const Option* option) {
	if (option == &Option::NAME) return username;
	return "";
}
