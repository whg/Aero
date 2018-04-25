//
// Created by whg on 22/04/18.
//

#include "FrameSequence.hpp"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"

#include "Output.hpp"

using namespace ci;
using namespace std;

FrameSequenceRef FrameSequence::create( const ci::fs::path &imageFolder, const Mapping &mapping, float frameRate ) {

	auto output = std::make_shared<FrameSequence>();
	output->setup( imageFolder, mapping, frameRate );
	return output;
}

void FrameSequence::setup( const ci::fs::path &imageFolder, const Mapping &mapping, float frameRate ) {

	std::vector<std::string> filepaths;
	for ( fs::directory_iterator it( imageFolder ); it != fs::directory_iterator(); ++it ) {
		if ( it->path().extension().string().find( "png" ) != std::string::npos ) {
			filepaths.emplace_back( it->path().string() );
		}
	}

	std::sort( filepaths.begin(), filepaths.end() );

	mDmxSequences.clear();
	for ( const auto &filepath : filepaths ) {
		auto channel = Channel( loadImage( filepath ) );
		mDmxSequences.emplace_back( mapping.dmxFromChannel( channel ) );
	}

	mFrameRate = frameRate;
	mDisplayHeight = mapping.getMaxDmxChannel();

	mTexture = generateTexture();
	mAllowsMute = true;
}

float FrameSequence::getDuration() const {
	return mDmxSequences.size() / mFrameRate;
}

int FrameSequence::getHeight() const {
	return mTexture->getHeight();
}

void FrameSequence::draw( int width ) const {
	gl::enableAlphaBlending();
	Rectf rect( 0, 0, width,  mTexture->getHeight() );

	gl::draw( mTexture, rect );

//	gl::colorMask( 1, 1, 1, 0 );
	gl::ScopedBlend blend( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );

//	gl::ScopedColor c( 1, 1, 1 );
//	gl::drawSolidRect( rect );

}

gl::TextureRef FrameSequence::generateTexture() {
	Channel channel( mDmxSequences.size(), mDisplayHeight );

	for ( size_t col = 0; col < channel.getWidth(); col++ ) {
		const auto &sequence = mDmxSequences[col];
		auto chanIter = channel.getIter( ci::Area( col, 0, col + 1, mDisplayHeight ) );
		size_t i = 0;
		while ( chanIter.line() ) {
			while ( chanIter.pixel() ) {
				chanIter.v() = sequence[i++];
			}
		}
	}

	auto tex =  gl::Texture::create( channel );
	tex->setMagFilter( GL_NEAREST );
	return tex;
}

void FrameSequence::writeData( size_t frameNum ) const {
	if ( frameNum >= mDmxSequences.size() ) {
		CI_LOG_E( frameNum << "is too big an index" );
		return;
	}

	if ( frameNum >= mMuteUntilFrame ) {
		Output::get()->setValues( mDmxSequences[frameNum] );
	}

}



