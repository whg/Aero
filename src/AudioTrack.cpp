//
// Created by whg on 23/04/18.
//

#include "AudioTrack.hpp"

// not sure why this isn't linking
namespace cinder { namespace audio {
std::string Node::getName() const { return ""; }
} }

using namespace std;
using namespace ci;

AudioTrack::AudioTrack(): mDisplaySize( 1440, 100 ) {

}

AudioTrackRef AudioTrack::create( const ci::fs::path &path, int height ) {

	auto output = std::make_shared<AudioTrack>();

	auto sourceFile = audio::load( loadFile( path ) );
	auto buffer = sourceFile->loadBuffer();

	auto ctx = audio::Context::master();
	output->mBufferPlayer = ctx->makeNode( new audio::BufferPlayerNode( buffer ) );
	output->mBufferPlayer->stop();
	output->mGain = ctx->makeNode( new audio::GainNode( 0.5f ) );
	output->mBufferPlayer >> output->mGain >> ctx->getOutput();

	output->mDisplaySize.y = height;
	output->generateTexture( buffer );
	output->mBufferPlayer->stop();
	output->mBufferPlayer->disable();

	return output;
}

void AudioTrack::play() {
	mBufferPlayer->start();
}

void AudioTrack::stop() {
	mBufferPlayer->stop();
}

void AudioTrack::setPlayhead( float t ) {
	mBufferPlayer->seekToTime( t );
}

float AudioTrack::getDuration() const {
	return mBufferPlayer->getNumSeconds();
}

int AudioTrack::getHeight() const {
	return mDisplaySize.y;
}

void AudioTrack::draw( int width ) const {
	gl::draw( mTexture, Rectf( 0, 0, width, mTexture->getHeight() ) );
}

void AudioTrack::generateTexture( const ci::audio::BufferRef &buffer ) {

	gl::FboRef fbo = gl::Fbo::create( mDisplaySize.x, mDisplaySize.y, true );

	{
		gl::enableAlphaBlending();
		gl::ScopedFramebuffer fbScp( fbo );
		gl::clear( ColorA( 0, 0, 0, 0 ) );

		gl::ScopedViewport scpVp( ivec2( 0 ), fbo->getSize() );
		gl::ScopedMatrices m;
		gl::setMatricesWindow( fbo->getSize() );

		gl::ScopedColor c( ColorA( 1, 1, 1, 0.8 ) );

		PolyLine2f line;
		const auto N = buffer->getNumFrames();
		float xStep = mDisplaySize.x / static_cast<float>( N );

		float *data = buffer->getChannel( 0 );
		float center = mDisplaySize.y * 0.5f;
		float height = mDisplaySize.y * 0.45f;
		float x = 0;
		for ( int i = 0; i < N; i++ ) {
			line.push_back( vec2( x, center + data[i] * height ) );
			x+= xStep;
		}

		gl::draw( line );
	}

	auto surface = fbo->readPixels8u( Area( 0, 0, fbo->getWidth(), fbo->getHeight() ) );
	mTexture = gl::Texture::create( surface );

}




