//
// Created by whg on 23/04/18.
//

#include <chrono>

#include <cinder/gl/gl.h>
#include "Transport.hpp"

#include "CinderImGui.h"


using namespace ci;
using namespace std;

Transport::Transport(): mDisplaySize( 640, 0 ), mDuration( 0.f ), mCueFrame( 0 ),
						mFrameRate( 25 ), mPlaying( false ), mPlayhead( 0 ),
						mFrameNumber( 0 ), mEndFrame( 0 ) {

}


void Transport::add( TransportObjectRef object ) {
	mObjects.emplace_back( object );
	mDisplaySize.y+= object->getHeight();
	mDuration = std::max( object->getDuration(), mDuration );
	if ( object->isFrameBased() ) {
		mEndFrame = std::max( object->getNumFrames(), mEndFrame );
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
				gl::ScopedColor col( 0.0, 0, 0.25, 0.9 );
				auto end = mDisplaySize.x * ( object->getMuteUntil() / mDuration );
				gl::drawSolidRect( Rectf( 0, 0, end, h ) );
			}

			gl::translate( 0, h );
		}
	}

	gl::ScopedColor c( 1, 0, 0 );
	auto x = timeToScreen( mPlayhead.load() );
	gl::drawLine( vec2( x, 0 ), vec2( x, mDisplaySize.y ) );
}

void Transport::drawUi() {
	ui::ScopedWindow window( "Transport" );
	int frame = static_cast<int>( mFrameNumber.load() );
	float time = frame / DEFAULT_FRAMERATE;
	ui::DragInt( "Frame", &frame );
	ui::DragFloat( "Time", &time );
	ui::DragFloat( "Frame rate", &mFrameRate, 0.1f, 1 );
	ui::DragInt( "Cue point", &mCueFrame, 0.1f, 0 );

	for ( const auto &object : mObjects ) {
		auto mu = object->getMuteUntilFrame();
		if ( ui::DragInt( ( object->getName() + " mu" ).c_str(), &mu, 0 ) ) {
			object->setMuteUntilFrame( mu );
		}
	}
}

void Transport::play() {

	mFrameNumber = mCueFrame;
	mPlayhead.store( mFrameNumber / DEFAULT_FRAMERATE );
	mPlaying.store( true );

	mPlayThread = std::thread( &Transport::update, this );

	for ( auto &object : mObjects ) {
		object->play();
		object->setPlayhead( mPlayhead.load() );
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

		std::this_thread::sleep_for( chrono::duration<float>( frameTime ) );
	}
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





