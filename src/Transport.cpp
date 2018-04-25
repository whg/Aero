//
// Created by whg on 23/04/18.
//

#include <chrono>

#include <cinder/gl/gl.h>
#include "Transport.hpp"

#include "CinderImGui.h"
#include "Output.hpp"


using namespace ci;
using namespace std;

Transport::Transport(): mDisplaySize( 640, 0 ), mDuration( 0.f ), mCueFrame( 0 ),
						mFrameRate( 25 ), mPlaying( false ), mPlayhead( 0 ),
						mFrameNumber( 0 ), mEndFrame( 0 ), mStopFrame( 0 ) {

}


void Transport::add( TransportObjectRef object ) {
	mObjects.emplace_back( object );
	mDisplaySize.y+= object->getHeight();
	mDuration = std::max( object->getDuration(), mDuration );
	if ( object->isFrameBased() ) {
		mEndFrame = std::max( object->getNumFrames(), mEndFrame );
		mStopFrame = mEndFrame;
	}
}

void Transport::remove( std::string name ) {
	for ( auto it = mObjects.begin(); it != mObjects.end(); ++it ) {
		if ( (*it)->getName() == name ) {
			mObjects.erase( it );
			break;
		}
	}
}

void Transport::replace( std::string name, TransportObjectRef object ) {
	for ( auto it = mObjects.begin(); it != mObjects.end(); ++it ) {
		if ( (*it)->getName() == name ) {
			*it = object;
			break;
		}
	}
}

void Transport::draw() {

	{
		gl::ScopedMatrices m;
		for ( const auto &object : mObjects ) {
			object->draw( static_cast<int>( mDisplaySize.x * ( object->getDuration() / mDuration ) ) );
			int h = object->getHeight();

			if ( object->getMuteUntil() > 0.f )  {
				gl::ScopedColor col( 0.1843, 0.0549, 0.1059, 0.75 );
				auto end = mDisplaySize.x * ( object->getMuteUntil() / mDuration );
				gl::drawSolidRect( Rectf( 0, 0, end, h ) );
			}

			gl::translate( 0, h );
		}


	}

	gl::ScopedColor c( 1, 0, 0 );
	auto x = timeToScreen( mPlayhead.load() );
	gl::drawLine( vec2( x, 0 ), vec2( x, mDisplaySize.y ) );

	gl::enableAlphaBlending();
	gl::color( 0, 0, 0, 0.75 );
	float cueX = timeToScreen( mCueFrame / DEFAULT_FRAMERATE );
	gl::drawSolidRect( Rectf( 0, 0, cueX, mDisplaySize.y ) );

	float stopX = timeToScreen( mStopFrame / DEFAULT_FRAMERATE );
	gl::drawSolidRect( Rectf( stopX, 0, mDisplaySize.x, mDisplaySize.y ) );

}

void Transport::drawUi() {
	ui::ScopedWindow window( "Transport" );
	int frame = static_cast<int>( mFrameNumber.load() );
	float time = frame / DEFAULT_FRAMERATE;
	ui::DragInt( "Frame", &frame );
	ui::DragFloat( "Time", &time );
	ui::DragFloat( "Frame rate", &mFrameRate, 0.1f, 1, 100 );
	ui::DragInt( "Cue frame", &mCueFrame, 0.5f, 0, mStopFrame );
	ui::DragInt( "Stop frame", &mStopFrame, 0.5f, mCueFrame, mEndFrame );

	for ( const auto &object : mObjects ) {
		if ( object->getAllowsMute() ) {
			auto mu = object->getMuteUntilFrame();
			if ( ui::DragInt( ( object->getName() + " mute" ).c_str(), &mu, 0.5f, 0, mEndFrame ) ) {
				object->setMuteUntilFrame( mu );
			}
		}
	}
}

void Transport::play() {

	mFrameNumber = mCueFrame;
	mPlayhead.store( mFrameNumber / DEFAULT_FRAMERATE );
	mPlaying.store( true );



	if ( mPlayThread.joinable() ) {
		mPlayThread.join();
	}

	mPlayThread = std::thread( &Transport::update, this );

	for ( auto &object : mObjects ) {
		float speed = mFrameRate / DEFAULT_FRAMERATE;
		object->setSpeed( speed );
		object->play();
		object->setPlayhead( mPlayhead.load() / speed );
	}
}

void Transport::pause() {

}

void Transport::stop() {

	for ( auto &object : mObjects ) {
		object->stop();
	}

	mPlaying.store( false );
	mPlayThread.join();
}

void Transport::update() {
	float frameTime = 1.f / mFrameRate;

	while ( mPlaying ) {

		for ( const auto &object : mObjects ) {
			if ( object->isFrameBased() && mFrameNumber.load() < object->getNumFrames() ) {
				object->writeData( mFrameNumber.load() );
			}
		}

		mFrameNumber.store( mFrameNumber.load() + 1 );
		mPlayhead.store( mFrameNumber.load() / DEFAULT_FRAMERATE );


		if ( mFrameNumber.load() >= mEndFrame ) {
			break;
		}
		else if ( mFrameNumber.load() >= mStopFrame ) {
			for ( auto &object : mObjects ) {
				object->stop();
			}
			break;
		}

		std::this_thread::sleep_for( chrono::duration<float>( frameTime ) );
	}

	Output::get()->setValues( 0 );
	mPlaying.store( false );

}

int Transport::getHeight() const {
	int height = 0;
	for ( const auto &object : mObjects ) {
		height+= object->getHeight();
	}
	return height;
}

void Transport::mouseDown( glm::ivec2 p ) {
	if ( p.y < getHeight() ) {

		if ( mPlayThread.joinable() ) {
			stop();
		}

		float t = screenToTime( p.x );
		mCueFrame = static_cast<int>( t * DEFAULT_FRAMERATE );
		play();
	}
}





