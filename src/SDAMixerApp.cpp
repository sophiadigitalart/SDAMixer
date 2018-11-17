#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

// Settings
#include "SDASettings.h"
// Session
#include "SDASession.h"
// Log
#include "SDALog.h"
// Spout
#include "CiSpoutOut.h"
// UserInterface
//#include "CinderImGui.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace SophiaDigitalArt;

class SDAMixerApp : public App {

public:
	SDAMixerApp();
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
	void update() override;
	void draw() override;
	void cleanup() override;
	void setUIVisibility(bool visible);
private:
	// Settings
	SDASettingsRef					mSDASettings;
	// Session
	SDASessionRef					mSDASession;
	// Log
	SDALogRef						mSDALog;
	// imgui
	float							color[4];
	float							backcolor[4];
	int								playheadPositions[12];
	int								speeds[12];

	float							f = 0.0f;
	char							buf[64];
	unsigned int					i, j;

	bool							mouseGlobal;

	string							mError;
	// fbo
	bool							mIsShutDown;
	Anim<float>						mRenderWindowTimer;
	void							positionRenderWindow();
	bool							mFadeInDelay;
	SpoutOut 						mSpoutOut;
};


SDAMixerApp::SDAMixerApp()
	: mSpoutOut("SDAMixer", app::getWindowSize())
{
	// Settings
	mSDASettings = SDASettings::create();
	// Session
	mSDASession = SDASession::create(mSDASettings);
	//mSDASettings->mCursorVisible = true;
	setUIVisibility(mSDASettings->mCursorVisible);
	mSDASession->getWindowsResolution();

	mouseGlobal = false;
	mFadeInDelay = true;
	// windows
	mIsShutDown = false;
	//mRenderWindowTimer = 0.0f;
	//timeline().apply(&mRenderWindowTimer, 1.0f, 2.0f).finishFn([&] { positionRenderWindow(); });

}
void SDAMixerApp::positionRenderWindow() {
	mSDASession->getWindowsResolution();
	mSDASettings->mRenderPosXY = ivec2(mSDASettings->mRenderX, mSDASettings->mRenderY);//20141214 was 0
	setWindowPos(mSDASettings->mRenderX, mSDASettings->mRenderY);
	setWindowSize(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight);
}
void SDAMixerApp::setUIVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}
void SDAMixerApp::fileDrop(FileDropEvent event)
{
	mSDASession->fileDrop(event);
}
void SDAMixerApp::update()
{
	mSDASession->setFloatUniformValueByIndex(mSDASettings->IFPS, getAverageFps());
	mSDASession->update();
}
void SDAMixerApp::cleanup()
{
	if (!mIsShutDown)
	{
		mIsShutDown = true;
		CI_LOG_V("shutdown");
		// save settings
		mSDASettings->save();
		mSDASession->save();
		quit();
	}
}
void SDAMixerApp::mouseMove(MouseEvent event)
{
	if (!mSDASession->handleMouseMove(event)) {
		// let your application perform its mouseMove handling here
	}
}
void SDAMixerApp::mouseDown(MouseEvent event)
{
	if (!mSDASession->handleMouseDown(event)) {
		// let your application perform its mouseDown handling here
		if (event.isRightDown()) { 
		}
	}
}
void SDAMixerApp::mouseDrag(MouseEvent event)
{
	if (!mSDASession->handleMouseDrag(event)) {
		// let your application perform its mouseDrag handling here
	}	
}
void SDAMixerApp::mouseUp(MouseEvent event)
{
	if (!mSDASession->handleMouseUp(event)) {
		// let your application perform its mouseUp handling here
	}
}

void SDAMixerApp::keyDown(KeyEvent event)
{
	if (!mSDASession->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_KP_PLUS:
		case KeyEvent::KEY_DOLLAR:
		case KeyEvent::KEY_TAB:
			positionRenderWindow();
		case KeyEvent::KEY_ESCAPE:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_h:
			// mouse cursor and ui visibility
			mSDASettings->mCursorVisible = !mSDASettings->mCursorVisible;
			setUIVisibility(mSDASettings->mCursorVisible);
			break;
		}
	}
}
void SDAMixerApp::keyUp(KeyEvent event)
{
	if (!mSDASession->handleKeyUp(event)) {
	}
}

void SDAMixerApp::draw()
{
	gl::clear(Color::black());
	if (mFadeInDelay) {
		mSDASettings->iAlpha = 0.0f;
		if (getElapsedFrames() > mSDASession->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mSDASettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}

	//gl::setMatricesWindow(toPixels(getWindowSize()),false);
	gl::setMatricesWindow(mSDASettings->mRenderWidth, mSDASettings->mRenderHeight, false);
	gl::draw(mSDASession->getMixTexture(), getWindowBounds());
	gl::drawString("fps: " + std::to_string((int)getAverageFps()), vec2(getWindowWidth() - toPixels(100), toPixels(20)), Color(1, 1, 1), Font("Verdana", toPixels(24)));
	//for (unsigned int f = 0; f < mSDASession->getFboListSize(); f++) {
		//gl::draw(mSDASession->getFboRenderedTexture(mSDASession->getFboRenderedTexture(f)->getId()), Area(0, 0, f * mSDASettings->mPreviewFboWidth, f * mSDASettings->mPreviewFboHeight));
	for (unsigned int f = 0; f < mSDASession->getFboListSize() - 1; f++) {
		gl::draw(mSDASession->getFboRenderedTexture(f), Area(0, 0, f *  mSDASettings->mPreviewFboWidth, f * mSDASettings->mPreviewFboHeight));
	}
	// Spout Send
	mSpoutOut.sendViewport();
	getWindow()->setTitle(mSDASettings->sFps + " fps SDAMixer");
}

void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(640, 480);
}

CINDER_APP(SDAMixerApp, RendererGl, prepareSettings)
