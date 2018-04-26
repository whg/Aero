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

class AeroApp : public App {
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

	bool loadSequence( std::string folder );
	std::string mSequenceFolder;

	fs::path folderPathFromName( std::string name );
};

void AeroApp::setup() {
//    getWindow()->setTitle( "CinderApp" );

	ui::initialize( ui::Options().darkTheme() );

	mSequenceFolder.reserve( 100 );
	mSequenceFolder = "shoot_day_07_camera_protector";
	auto imageFolder = getAssetPath( mSequenceFolder );
	mMapping = std::make_shared<Mapping>( getAssetPath( "mapping.json" ) );
	mFrameSequence = FrameSequence::create( folderPathFromName( mSequenceFolder ), *mMapping, DEFAULT_FRAMERATE );
	mFrameSequence->setName( "Bubbles" );

	fs::path audioPath("/home/whg/Dropbox/sequences/audio/playback_audio_mono_click2.wav");
//	fs::path audioPath("/home/whg/Dropbox/sequences/audio/VUperc_mono.wav");
	mHouseAudio = AudioTrack::create( audioPath, 0 );
	mHouseAudio->setName( "House audio" );
	mHouseAudio->setAllowsMute( true );
//	mHouseAudio->setGain( 0.7f );

	mTimecode = AudioTrack::create( getAssetPath("timecode25-cue.wav"), 1 , 30 );
	mTimecode->setName( "Timecode" );
	mTimecode->setAllowsNegativeCue( true );
	mTimecode->setLeadinTime( 120 );
	mTimecode->generateTexture();
	mTimecode->setMuteUntilFrame( -30000  );

	mTransport.add( mFrameSequence );
	mTransport.add( mHouseAudio );
	mTransport.add( mTimecode );
	mTransport.setDisplayWidth( getWindowWidth() );

	mMappingOffset = ivec2( 20 , mTransport.getHeight() + 20 );

	mOutput = Output::get();
}

void AeroApp::quit() {

}

void AeroApp::mouseDown( MouseEvent event ) {
	mMapping->mouseDown( event.getPos() - mMappingOffset );
	mTransport.mouseDown( event.getPos() );
}

void AeroApp::mouseUp( MouseEvent event ) {
	mMapping->mouseUp( event.getPos() - mMappingOffset );
}

void AeroApp::keyDown( KeyEvent event ) {
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
	else if ( key == 'r' ) {
		loadSequence( mSequenceFolder );
	}
}

void AeroApp::draw() {
	gl::enableAlphaBlending();

    gl::clear( Color( 0.1f, 0.1f, 0.1f ) );

	mTransport.draw();
	mTransport.drawUi();

	gl::ScopedMatrices sm;
	gl::translate( mMappingOffset );
	mMapping->draw();
	mMapping->drawUi();

	mOutput->drawUi();

	ui::ScopedWindow window( "Main" );
	static double labelTimer = 0;
	static std::string label;
	if ( ui::InputText( "Sequence", &mSequenceFolder, ImGuiInputTextFlags_EnterReturnsTrue ) ) {
		if ( loadSequence( mSequenceFolder ) ) {
			label = "loaded";
		}
		else {
			label = "folder doesn't exist";
		}
		labelTimer = getElapsedSeconds();
	}

	if ( labelTimer > getElapsedSeconds() - 3 ) {
		ui::Text( label.c_str() );
	}
}

bool AeroApp::loadSequence( std::string folder ) {
	auto path = folderPathFromName( folder );
	if ( ! fs::exists( path ) ) {
		return false;
	}
	mFrameSequence->setup( path, *mMapping );
	return true;
}

fs::path AeroApp::folderPathFromName( std::string name ) {
	fs::path base( "/home/whg/Dropbox/sequences/trigger sequence" );
	return base / name;
}

CINDER_APP( AeroApp, RendererGl )
