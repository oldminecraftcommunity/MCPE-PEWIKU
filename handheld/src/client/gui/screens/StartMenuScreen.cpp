#include "StartMenuScreen.h"
#include "SelectWorldScreen.h"
#include "ProgressScreen.h"
#include "JoinGameScreen.h"
#include "OptionsScreen.h"
#include "PauseScreen.h"
#include "InvalidLicenseScreen.h"
//#include "BuyGameScreen.h"

#include "../../../SharedConstants.h"
#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../../AppPlatform.h"
#include "../components/ImageButton.h"
#include "../components/NinePatch.h"
#include "../../../util/Mth.h"
#include "../../../util/Random.h"
#include "../../../util/Common.h"
#include "../../../locale/I18n.h"
#include <cmath>
int StartMenuScreen::currentSplash = -1;

SettingsButton::SettingsButton(int id, const std::string& msg)
	: ImageButton(id, msg)
	, normalBg(NULL)
	, pressedBg(NULL) {
}

SettingsButton::~SettingsButton() {
	if (normalBg != NULL) {
		delete normalBg;
		normalBg = NULL;
	}
	if (pressedBg != NULL) {
		delete pressedBg;
		pressedBg = NULL;
	}
}

void SettingsButton::setBackgrounds(NinePatchLayer* normalBg, NinePatchLayer* pressedBg) {
	if (this->normalBg != NULL) {
		delete this->normalBg;
	}
	if (this->pressedBg != NULL) {
		delete this->pressedBg;
	}
	this->normalBg = normalBg;
	this->pressedBg = pressedBg;
}

void SettingsButton::renderBg(Minecraft* minecraft, int xm, int ym) {
	NinePatchLayer* layer = normalBg;
	if ((active && _currentlyDown && hovered(minecraft, xm, ym)) || selected) {
		layer = pressedBg;
	}
	if (layer != NULL) {
		layer->setSize((float)width, (float)height);
		layer->draw(Tesselator::instance, (float)x, (float)y);
	}
}

static const char* gSplashes[] = {
	"Scientific!", "Cooler than Spock!", "Collaborate and listen!", "Never dig down!",
	"Take frequent breaks!", "Not linear!", "Han shot first!", "Nice to meet you!",
	"Buckets of lava!", "Ride the pig!", "Larger than Earth!", "sqrt(-1) love you!",
	"Phobos anomaly!", "Punching wood!", "Falling off cliffs!", "0% sugar!",
	"150% hyperbole!", "Synecdoche!", "Let's danec!", "Seecret Friday update!",
	"Ported implementation!", "Lewd with two dudes with food!", "Kiss the sky!",
	"20 GOTO 10!", "Verlet intregration!", "Peter Griffin!", "Do not distribute!",
	"Cogito ergo sum!", "4815162342 lines of code!", "A skeleton popped out!",
	"The Work of Notch!", "The sum of its parts!", "BTAF used to be good!",
	"I miss ADOM!", "umop-apisdn!", "OICU812!", "Bring me Ray Cokes!",
	"Finger-licking!", "Thematic!", "Pneumatic!", "Sublime!", "Octagonal!",
	"Une baguette!", "Gargamel plays it!", "Rita is the new top dog!",
	"SWM forever!", "Representing Edsbyn!", "Matt Damon!",
	"Supercalifragilisticexpialidocious!", "Consummate V's!", "Cow Tools!",
	"Double buffered!", "V-synched!", "Fan fiction!", "Flaxkikare!",
	"Jason! Jason! Jason!", "Hotter than the sun!", "Internet enabled!",
	"Autonomous!", "Engage!", "Fantasy!", "DRR! DRR! DRR!", "Kick it root down!",
	"Regional resources!", "Woo, facepunch!", "Woo, somethingawful!", "Woo, /v/!",
	"Woo, tigsource!", "Woo, minecraftforum!", "Woo, worldofminecraft!",
	"Woo, reddit!", "Woo, 2pp!", "Google anlyticsed!", "Give us Gordon!",
	"Tip your waiter!", "Very fun!", "12345 is a bad password!",
	"Vote for net neutrality!", "Lives in a pineapple under the sea!",
	"Omnipotent!", "Gasp!", "...!", "Bees, bees, bees, bees!", "Haha, LOL!",
	"Hampsterdance!", "Switches and ores!", "Menger sponge!", "idspispopd!",
	"Eple (original edit)!", "So fresh, so clean!", "Don't look directly at the bugs!",
	"Oh, ok, Pigmen!", "Scary!", "Play Minecraft, Watch Topgear, Get Pig!",
	"Twittered about!", "Jump up, jump up, and get down!", "Joel is neat!",
	"A riddle, wrapped in a mystery!", "Huge tracts of land!", "Welcome to your Doom!",
	"Stay a while, stay forever!", "Stay a while and listen!", "Treatment for your rash!",
	"\"Autological\" is!", "Information wants to be free!",
	"\"Almost never\" is an interesting concept!", "Lots of truthiness!",
	"The creeper is a spy!", "It's groundbreaking!", "Let our battle's begin!",
	"The sky is the limit!", "Jeb has amazing hair!", "Casual gaming!",
	"Undefeated!", "Kinda like Lemmings!", "Follow the train, CJ!",
	"Leveraging synergy!", "DungeonQuest is unfair!", "110813!", "90210!",
	"Tyrion would love it!", "Also try VVVVVV!", "Also try Super Meat Boy!",
	"Also try Terraria!", "Also try Mount And Blade!", "Also try Project Zomboid!",
	"Also try World of Goo!", "Also try Limbo!", "Also try Pixeljunk Shooter!",
	"Also try Braid!", "That's super!", "Bread is pain!", "Read more books!",
	"Khaaaaaaaaan!", "Less addictive than TV Tropes!", "More addictive than lemonade!",
	"Bigger than a bread box!", "Millions of peaches!", "Fnord!",
	"This is my true form!", "Totally forgot about Dre!", "Don't bother with the clones!",
	"Pumpkinhead!", "Hobo humping slobo babe!", "Endless!", "Feature packed!",
	"Boots with the fur!", "Stop, hammertime!", "Conventional!",
	"Homeomorphic to a 3-sphere!", "Doesn't avoid double negatives!",
	"Place ALL the blocks!", "Does barrel rolls!", "Meeting expectations!",
	"PC gaming since 1873!", "Ghoughpteighbteau tchoghs!", "Got your nose!",
	"Haley loves Elan!", "Afraid of the big, black bat!", "Doesn't use the U-word!",
	"Child's play!", "See you next Friday or so!", "150 bpm for 400000 minutes!",
	"Technologic!", "Funk soul brother!", "Pumpa kungen!", "Helo Cymru!",
	"My life for Aiur!", "Lennart lennart = new Lennart();",
	"I see your vocabulary has improved!", "Who put it there?",
	"You can't explain that!", "if not ok then return end",
	"SOPA means LOSER in Swedish!", "Big Pointy Teeth!", "Bekarton guards the gate!",
	"Mmmph, mmph!", "Don't feed avocados to parrots!", "Swords for everyone!",
	"Plz reply to my tweet!", ".party()!", "Take her pillow!", "Put that cookie down!",
	"Pretty scary!", "I have a suggestion.", "Now with extra hugs!",
	"Almost C++11!", "Woah.", "HURNERJSGER?", "What's up, Doc?",
	"1 star! Deal with it notch!", "100% more yellow text!", "Glowing creepy eyes!",
	"Toilet friendly!", "Annoying touch buttons!", "Astronomically accurate!",
	"0xffff-1 chunks!", "Cubism!", "Pocket!", "Mostly harmless!", "!!!1!",
	"Dramatic lighting!", "As seen on TV!", "Awesome!", "100% pure!",
	"May contain nuts!", "Better than Prey!", "Less polygons!", "Sexy!",
	"Limited edition!", "Flashing letters!", "Made by Mojang!", "It's here!",
	"Best in class!", "It's alpha!", "100% dragon free!", "Excitement!",
	"More than 500 sold!", "One of a kind!", "Heaps of hits on YouTube!",
	"Indev!", "Spiders everywhere!", "Check it out!", "Holy cow, man!",
	"It's a game!", "Made in Sweden!", "Uses C++!", "Reticulating splines!",
	"Minecraft!", "Yaaay!", "Multiplayer!", "Touch compatible!", "Undocumented!",
	"Ingots!", "Exploding creepers!", "That's no moon!", "l33t!", "Create!",
	"Survive!", "Dungeon!", "Exclusive!", "The bee's knees!", "Down with O.P.P.!",
	"Closed source!", "Classy!", "Wow!", "Not on steam!", "Oh man!",
	"Awesome community!", "Pixels!", "Teetsuuuuoooo!", "Kaaneeeedaaaa!",
	"Enhanced!", "90% bug free!", "Pretty!", "12 herbs and spices!", "Fat free!",
	"Absolutely no memes!", "Free dental!", "Ask your doctor!", "Minors welcome!",
	"Cloud computing!", "Legal in Finland!", "Hard to label!", "Technically good!",
	"Bringing home the bacon!", "Quite Indie!", "GOTY!", "Euclidian!", "Now in 3D!",
	"Inspirational!", "Herregud!", "Complex cellular automata!", "Yes, sir!",
	"Played by cowboys!", "OpenGL ES 1.1!", "Thousands of colors!", "Try it!",
	"Age of Wonders is better!", "Try the mushroom stew!", "Sensational!",
	"Hot tamale, hot hot tamale!", "Play him off, keyboard cat!", "Guaranteed!",
	"Macroscopic!", "Bring it on!", "Random splash!", "Call your mother!",
	"Monster infighting!", "Loved by millions!", "Ultimate edition!", "Freaky!",
	"You've got a brand new key!", "Water proof!", "Uninflammable!", "Whoa, dude!",
	"All inclusive!", "Tell your friends!", "NP is not in P!", "Notch <3 ez!",
	"Livestreamed!", "Haunted!", "Polynomial!", "Terrestrial!", "All is full of love!",
	"Full of stars!", NULL
};

StartMenuScreen::StartMenuScreen()
	: Screen()
	, playButton(2, 0, 0, 100, 30, "")
	, playMultiplayer(3, 0, 0, 100, 30, "")
	, settingsButton(4, "")
	, buyButton(5, 0, 0, 210, 24, "") {
	copyrightString = "";
	gameVersion = "";
	
	if (currentSplash == -1) {
		int numSplashes = 0;
		while (gSplashes[numSplashes] != NULL) numSplashes++;
		currentSplash = Random(Common::getRawTimeS()).nextInt(numSplashes);
	}
	splash = gSplashes[currentSplash];
}

StartMenuScreen::~StartMenuScreen() {
}

void StartMenuScreen::init() {
	playButton.msg = "Play";
	playMultiplayer.msg = "Multiplayer";
	
	ImageDef def;
	def.name = "gui/touchgui.png";
	def.setSrc(IntRectangle(218, 0, 22, 21));
	def.x = 5;
	def.y = 5;
	def.width = 22;
	def.height = 21;
	settingsButton.setImageDef(def, false);
	settingsButton.width = 32;
	settingsButton.height = 32;
	NinePatchFactory settingsBg(minecraft->textures, "gui/spritesheet.png");
	settingsButton.setBackgrounds(
		settingsBg.createSymmetrical(IntRectangle(112, 0, 8, 67), 2, 2, 32.0f, 32.0f),
		settingsBg.createSymmetrical(IntRectangle(120, 0, 8, 67), 2, 2, 32.0f, 32.0f));

	buttons.push_back(&playButton);
	buttons.push_back(&playMultiplayer);
	buttons.push_back(&settingsButton);
	buttons.push_back(&buyButton);

	buyButton.visible = false;
	
	setupPositions();

	copyrightString = "\xffMojang AB";
	gameVersion = Common::getGameVersionString();
	#if defined(DEBUG)
		gameVersion = gameVersion + " [DEBUG]";
	#endif

	settingsButton.active = true;
	settingsButton.visible = true;
	playButton.active = true;
	playMultiplayer.active = true;
}

void StartMenuScreen::setupPositions() {
	int midX = width / 2;
	int midY = height / 2;
	
	playButton.width = 100;
	playButton.height = 30;
	playMultiplayer.width = 100;
	playMultiplayer.height = 30;
	
	playButton.x = midX - 50;
	playButton.y = midY - 15;
	
	playMultiplayer.x = midX - 50;
	playMultiplayer.y = midY + 25;

	settingsButton.x = width - settingsButton.width - 2;
	settingsButton.y = height - settingsButton.height - 2;

	copyrightPosX = 2;
	versionPosX = 2;
}

void StartMenuScreen::tick() {
	_updateLicense();
}

void StartMenuScreen::render(int xm, int ym, float a) {
	_updateLicense();
	renderMenuBackground(a);
	
	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	const TextureData* td = minecraft->textures->getTemporaryTextureData(minecraft->textures->loadTexture("gui/title.png"));
	if (td) {
		minecraft->textures->bind(minecraft->textures->loadTexture("gui/title.png"));
		
		float texWidth = (float)td->w;
		float halfScreenWidth = (float)width * 0.5f;
		float halfRenderWidth = texWidth * 0.5f;
		
		if (halfScreenWidth <= halfRenderWidth) {
			halfRenderWidth = halfScreenWidth;
		}
		
		float scale = (halfRenderWidth + halfRenderWidth) / texWidth;
		float renderHeight = (float)td->h * scale;
		
		glColor4f2(1.0f, 1.0f, 1.0f, 1.0f);
		Tesselator& t = Tesselator::instance;
		t.begin();
		float xLeft = halfScreenWidth - halfRenderWidth;
		float xRight = halfScreenWidth + halfRenderWidth;
		float yTop = 4.0f;
		float yBottom = renderHeight + 4.0f;
		
		t.vertexUV(xLeft, yBottom, blitOffset, 0.0f, 1.0f);
		t.vertexUV(xRight, yBottom, blitOffset, 1.0f, 1.0f);
		t.vertexUV(xRight, yTop, blitOffset, 1.0f, 0.0f);
		t.vertexUV(xLeft, yTop, blitOffset, 0.0f, 0.0f);
		t.draw();

		// Render Splash
		glPushMatrix2();
		glTranslatef2(xRight - 10, yBottom - 10, 0.0f);
		glRotatef2(-20.0f, 0.0f, 0.0f, 1.0f);
		
		float splashScale = std::pow(std::sin(minecraft->guiTime * 5.0f), 4.0f) * 0.1f + 1.2f;
		
		glScalef2(splashScale, splashScale, splashScale);
		drawCenteredString(minecraft->font, splash, 0, -8, 0xffffff00);
		glPopMatrix2();
	}
	
	glEnable2(GL_TEXTURE_2D);
	drawString(minecraft->font, gameVersion, versionPosX, height - 20, 0xffffffff);
	drawString(minecraft->font, copyrightString, copyrightPosX, height - 10, 0xffffffff);
	
	Screen::render(xm, ym, a);
}

void StartMenuScreen::buttonClicked(Button* button) {
	if (button->id == playButton.id) {
		minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
	} else if (button->id == playMultiplayer.id) {
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	} else if (button->id == settingsButton.id) {
		minecraft->setScreen(new OptionsScreen(0));
	} else if (button->id == buyButton.id) {
		minecraft->platform()->buyGame();
		//minecraft->setScreen(new BuyGameScreen());
	}
}

void StartMenuScreen::_updateLicense() {
	int licenseId = minecraft->getLicenseId();
	bool active = false;
	
	if (licenseId < 0) {
		active = false;
	} else if (licenseId > 1) {
		bool hasBuy = minecraft->platform()->hasBuyButtonWhenInvalidLicense();
		minecraft->setScreen(new InvalidLicenseScreen(licenseId, hasBuy));
		return;
	} else {
		active = true;
	}
	
	settingsButton.active = active;
	playButton.active = active;
	playMultiplayer.active = active;
}

bool StartMenuScreen::handleBackEvent(bool isDown) {
	minecraft->quit();
	return true;
}

bool StartMenuScreen::isInGameScreen() {
	return false;
}
