#include "Screen.h"
#include "components/Button.h"
#include "components/TextBox.h"
#include "../Minecraft.h"
#include "../renderer/Tesselator.h"
#include "../sound/SoundEngine.h"
#include "../../platform/input/Keyboard.h"
#include "../../platform/input/Mouse.h"
#include "../renderer/Textures.h"
#include "../renderer/gles.h"
#include "../renderer/states/DisableState.h"
#include "../renderer/states/EnableState.h"
#include "../../util/Mth.h"

static const char* panorama_images[] = {
	"gui/background/panorama_0.png",
	"gui/background/panorama_1.png",
	"gui/background/panorama_2.png",
	"gui/background/panorama_3.png",
	"gui/background/panorama_4.png",
	"gui/background/panorama_5.png"
};
static float panoramaTimer = 0.0f;

Screen::Screen()
:   passEvents(false),
	clickedButton(NULL),
	tabButtonIndex(0),
	width(1),
	height(1),
	minecraft(NULL),
	font(NULL)
{
}

void Screen::render( int xm, int ym, float a )
{
	for (unsigned int i = 0; i < buttons.size(); i++) {
		Button* button = buttons[i];
		button->render(minecraft, xm, ym);
	}
}

void Screen::init( Minecraft* minecraft, int width, int height )
{
	//particles = /*new*/ GuiParticles(minecraft);
	this->minecraft = minecraft;
	this->font = minecraft->font;
	this->width = width;
	this->height = height;
	init();
	setupPositions();
	updateTabButtonSelection();
}

void Screen::init()
{
}

void Screen::setSize( int width, int height )
{
	this->width = width;
	this->height = height;
	setupPositions();
}

bool Screen::handleBackEvent( bool isDown )
{
	return false;
}

void Screen::updateEvents()
{
	if (passEvents)
		return;

	while (Mouse::next())
		mouseEvent();

	while (Keyboard::next())
		keyboardEvent();
	while (Keyboard::nextTextChar())
		keyboardTextEvent();
}

void Screen::mouseEvent()
{
	// coords -> gui scale
	const MouseAction& e = Mouse::getEvent();
	int xm = e.x * width / minecraft->width;
	int ym = e.y * height / minecraft->height - 1;

	// update buttons when hovered
	if (e.action == MouseAction::ACTION_MOVE) {
		// for (unsigned int i = 0; i < tabButtons.size(); ++i) {
		// 	Button* button = tabButtons[i];
			
		// 	if (button->clicked(minecraft, xm, ym)) {
		// 		tabButtonIndex = i;
		// 		updateTabButtonSelection();
		// 		break;
		// 	}
		// }

		for (unsigned int i = 0; i < buttons.size(); ++i) {
			Button* button = buttons[i];
			// if hovered
			button->selected = button->clicked(minecraft, xm, ym);
		}
		return;
	}

	if (!e.isButton())
		return;
	
	// handle clicking
	if (Mouse::getEventButtonState()) {
		mouseClicked(xm, ym, Mouse::getEventButton());
	} else {
		mouseReleased(xm, ym, Mouse::getEventButton());
	}
}

void Screen::keyboardEvent()
{
	if (Keyboard::getEventKeyState()) {
		//if (Keyboard.getEventKey() == Keyboard.KEY_F11) {
		//    minecraft->toggleFullScreen();
		//    return;
		//}
		keyPressed(Keyboard::getEventKey());
	}
}
void Screen::keyboardTextEvent()
{
	keyboardNewChar(Keyboard::getChar());
}
void Screen::renderBackground()
{
	renderBackground(0);
}

void Screen::renderBackground( int vo )
{
	if (minecraft->isLevelGenerated()) {
		fillGradient(0, 0, width, height, 0xc0101010, 0xd0101010);
	} else {
		renderDirtBackground(vo);
	}
}

void Screen::renderDirtBackground( int vo )
{
	//glDisable2(GL_LIGHTING);
	glDisable2(GL_FOG);
	Tesselator& t = Tesselator::instance;
	minecraft->textures->loadAndBindTexture("gui/background.png");
	glColor4f2(1, 1, 1, 1);
	float s = 32;
	float fvo = (float) vo;
	t.begin();
	t.color(0x404040);
	t.vertexUV(0, (float)height, 0, 0, height / s + fvo);
	t.vertexUV((float)width, (float)height, 0, width / s, (float)height / s + fvo);
	t.vertexUV((float)width, 0, 0, (float)width / s, 0 + fvo);
	t.vertexUV(0, 0, 0, 0, 0 + fvo);
	t.draw();
}

void Screen::renderMenuBackground(float a)
{
	float time = (minecraft->guiTime * 30.0f) + a;
	for(int i = 0; i < 6; ++i) {
		minecraft->textures->loadTexture(panorama_images[i]);
	}

	glDisable2(GL_FOG);
	glDisable2(GL_CULL_FACE);
	glDisable2(GL_DEPTH_TEST);
	glDisable2(GL_BLEND);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable2(GL_TEXTURE_2D);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix2();
	glLoadIdentity2();
	gluPerspective(120.0f, 1.0f, 0.05f, 10.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix2();
	glLoadIdentity2();
	glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
	
	glRotatef2(180.0f, 1.0f, 0.0f, 0.0f);
	glRotatef2(Mth::sin(time / 400.0f) + 20.0f, 1.0f, 0.0f, 0.0f);
	glRotatef2(-time * 0.1f, 0.0f, 1.0f, 0.0f);
	
	for (int i = 0; i < 6; ++i) {
		glPushMatrix2();
		if (i == 1) glRotatef2(90.0f, 0.0f, 1.0f, 0.0f);
		if (i == 2) glRotatef2(180.0f, 0.0f, 1.0f, 0.0f);
		if (i == 3) glRotatef2(-90.0f, 0.0f, 1.0f, 0.0f);
		if (i == 4) glRotatef2(90.0f, 1.0f, 0.0f, 0.0f);
		if (i == 5) glRotatef2(-90.0f, 1.0f, 0.0f, 0.0f);

		minecraft->textures->loadAndBindTexture(panorama_images[i]);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		Tesselator& t = Tesselator::instance;
		t.begin(GL_QUADS);
		t.noColor();
		t.vertexUV(-1.0f, -1.0f, 1.0f, 0.0f, 0.0f);
		t.vertexUV(1.0f, -1.0f, 1.0f, 1.0f, 0.0f);
		t.vertexUV(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
		t.vertexUV(-1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		t.draw();
		
		glPopMatrix2();
	}
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix2();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix2();

	glEnable2(GL_CULL_FACE);
	glEnable2(GL_DEPTH_TEST);

	// Gradient overlay
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int startColor = (0x59 << 24) | (0xFF << 16) | (0xFF << 8) | 0xFF;
	int endColor   = (0x59 << 24) | 0;
	fillGradient(0, 0, width, height, startColor, endColor);
}

bool Screen::renderGameBehind(){
	return true; // minecraft->options.fancyGraphics;
}


bool Screen::isPauseScreen()
{
	return true;
}

bool Screen::isErrorScreen()
{
	return false;
}

bool Screen::isInGameScreen()
{
	return true;
}

bool Screen::closeOnPlayerHurt() {
    return false;
}

void Screen::keyPressed( int eventKey )
{
	if (eventKey == Keyboard::KEY_ESCAPE) {
		minecraft->setScreen(NULL);
		//minecraft->grabMouse();
	}
	if (minecraft->useTouchscreen())
		return;

	// "Tabbing" the buttons (walking with keys)
	const int tabButtonCount = tabButtons.size();
	if (!tabButtonCount)
		return;

	Options& o = minecraft->options;
	if (eventKey == o.keyMenuNext.key) {
		if (++tabButtonIndex == tabButtonCount) tabButtonIndex = 0;
		updateTabButtonSelection();
	} else if (eventKey == o.keyMenuPrevious.key) {
		if (--tabButtonIndex == -1) tabButtonIndex = tabButtonCount-1;
		updateTabButtonSelection();
	} else if (eventKey == o.keyMenuOk.key) {
		Button* button = tabButtons[tabButtonIndex];
		if (button->active) {
			minecraft->soundEngine->playUI("random.click", 1, 1);
			buttonClicked(button);
		}
	}
}

void Screen::updateTabButtonSelection()
{
	if (minecraft->useTouchscreen())
		return;

	for (unsigned int i = 0; i < tabButtons.size(); ++i)
		tabButtons[i]->selected = (i == tabButtonIndex);
}

void Screen::mouseClicked( int x, int y, int buttonNum )
{
	if (buttonNum == MouseAction::ACTION_LEFT) {
		for (unsigned int i = 0; i < buttons.size(); ++i) {
			Button* button = buttons[i];
            //LOGI("Hit-testing button: %p\n", button);
			if (button->clicked(minecraft, x, y)) {
                button->setPressed();

                //LOGI("Hit-test successful: %p\n", button);
				clickedButton = button;
/*
#if !defined(ANDROID) && !defined(__APPLE__) //if (!minecraft->isTouchscreen()) {
					minecraft->soundEngine->playUI("random.click", 1, 1);
					buttonClicked(button);
#endif }
*/
			}
		}
	}
}

void Screen::mouseReleased( int x, int y, int buttonNum )
{
	//LOGI("b_id: %d, (%p), text: %s\n", buttonNum, clickedButton, clickedButton?clickedButton->msg.c_str():"<null>");
	if (!clickedButton || buttonNum != MouseAction::ACTION_LEFT) return;

#if 1
//#if defined(ANDROID) || defined(__APPLE__) //if (minecraft->isTouchscreen()) {
		for (unsigned int i = 0; i < buttons.size(); ++i) {
			Button* button = buttons[i];
			if (clickedButton == button && button->clicked(minecraft, x, y)) {
				buttonClicked(button);
				minecraft->soundEngine->playUI("random.click", 1, 1);
				clickedButton->released(x, y);
			}
		}
# else //	} else {
		clickedButton->released(x, y);
#endif // }
	clickedButton = NULL;
}


bool Screen::hasClippingArea( IntRectangle& out )
{
	return false;
}

void Screen::lostFocus() {
	for(std::vector<TextBox*>::iterator it = textBoxes.begin(); it != textBoxes.end(); ++it) {
		TextBox* tb = *it;
		tb->loseFocus(minecraft);
	}
}

void Screen::toGUICoordinate( int& x, int& y ) {
	x = x * width / minecraft->width;
	y = y * height / minecraft->height - 1;
}
