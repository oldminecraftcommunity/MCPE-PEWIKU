#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__

#include "../Screen.h"
#include "../components/Button.h"

#include "../components/ImageButton.h"

class NinePatchLayer;

class SettingsButton: public ImageButton
{
public:
	SettingsButton(int id, const std::string& msg);
	virtual ~SettingsButton();

	void setBackgrounds(NinePatchLayer* normalBg, NinePatchLayer* pressedBg);

protected:
	void renderBg(Minecraft* minecraft, int xm, int ym);
	bool isSecondImage(bool hovered) { return false; }

private:
	NinePatchLayer* normalBg;
	NinePatchLayer* pressedBg;
};

class StartMenuScreen: public Screen
{
public:
	StartMenuScreen();
	virtual ~StartMenuScreen();

	void init();
	void setupPositions();

	void tick();
	void render(int xm, int ym, float a);

	void buttonClicked(Button* button);
	bool handleBackEvent(bool isDown);
	bool isInGameScreen();
private:
	void _updateLicense();

	Button playButton;
	Button playMultiplayer;
	SettingsButton settingsButton;
	Button buyButton;

	std::string copyrightString;
	int copyrightPosX;

	std::string gameVersion;
	int versionPosX;

	std::string splash;
	static int currentSplash;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__StartMenuScreen_H__*/
