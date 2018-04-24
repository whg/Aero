#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "CinderImGui.h"

#include "Mapping.hpp"
#include "FrameSequence.hpp"
#include "Output.hpp"
#include "AudioTrack.hpp"

using namespace ci;
using namespace ci::app;

class AreoApp : public App {
  public:
    void setup() override;
    void quit() override;
    void draw() override;

    void mouseDown( MouseEvent event ) override;
	void mouseUp( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;

	std::shared_ptr<Mapping> mMapping;
	FrameSequenceRef mFrameSequence;
	Transport mTransport;
	OutputRef mOutput;
	AudioTrackRef mHouseAudio;
	AudioTrackRef mTimecode;

	ivec2 mMappingOffset;
};

void AreoApp::setup() {
//    getWindow()->setTitle( "CinderApp" );
//	setWindowSize( 1000, 1000 );

	ui::initialize();

	auto imageFolder = getAssetPath( "test_002" );
	console() << imageFolder.string() << std::endl;
	mMapping = std::make_shared<Mapping>( getAssetPath( "mapping.json" ) );
	mFrameSequence = FrameSequence::create( imageFolder, *mMapping );
	mFrameSequence->setName( "Bubbles" );

	mHouseAudio = AudioTrack::create( getAssetPath( "house.wav" ), 0 );
	mHouseAudio->setName( "House audio" );

	mTimecode = AudioTrack::create( getAssetPath("timecode.wav"), 1, 30 );
	mTimecode->setName( "Timecode" );


	mTransport.add( mFrameSequence );
	mTransport.add( mHouseAudio );
	mTransport.add( mTimecode );
	mTransport.setDisplayWidth( getWindowWidth() );

//	mFrameSequence->setMuteUntilFrame( 340 );

	mMappingOffset = ivec2( 20 , mTransport.getHeight() + 20 );

	mOutput = Output::get();
}

void AreoApp::quit() {

}

void AreoApp::mouseDown( MouseEvent event ) {
	mMapping->mouseDown( event.getPos() - mMappingOffset );
	mTransport.mouseDown( event.getPos() );
}

void AreoApp::mouseUp( MouseEvent event ) {
	mMapping->mouseUp( event.getPos() - mMappingOffset );
}

void AreoApp::keyDown( KeyEvent event ) {
	auto key = event.getChar();
	if ( key == ' ' ) {
		if ( mTransport.isPlaying() ) {
			mTransport.stop();
			Output::get()->setValues( 0 );
		} else {
			mTransport.play();
		}
	}
	else if ( key == ']' ) {
		Output::get()->setValues( 0 );
	}
	else if ( key == '[' ) {
		Output::get()->setValues( 255 );
	}
}

void AreoApp::draw() {
	gl::enableAlphaBlending();

    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );

	mTransport.draw();
	mTransport.drawUi();

	gl::ScopedMatrices sm;
	gl::translate( mMappingOffset );
	mMapping->draw();
	mMapping->drawUi();

	mOutput->drawUi();

}

CINDER_APP( AreoApp, RendererGl )
