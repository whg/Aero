//
// Created by whg on 23/04/18.
//

#include <chrono>

#include <cinder/gl/gl.h>
#include "Transport.hpp"

#include "CinderImGui.h"


using namespace ci;
using namespace std;
using namespace std::chrono;

Transport::Transport(): mDisplaySize( 640, 0 ), mDuration( 0.f ), mCueFrame( 300 ),
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

void Transport::draw() {

	{
		gl::ScopedMatrices m;
		for ( const auto &object : mObjects ) {
			object->draw( mDisplaySize.x );
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
	int frame = mFrameNumber.load();
	ui::DragInt( "Frame", &frame );
	ui::DragFloat( "Frame rate", &mFrameRate, 0.1f, 1 );
	ui::DragInt( "Cue point", &mCueFrame, 0.1f, 0 );
}

void Transport::play() {

	mFrameNumber = mCueFrame;
	mPlayhead.store( DEFAULT_FRAMERATE * mFrameNumber );
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

		std::this_thread::sleep_for( duration<float>( frameTime ) );
	}
}

int Transport::getHeight() const {
	int height = 0;
	for ( const auto &object : mObjects ) {
		height+= object->getHeight();
	}
	return height;
}

