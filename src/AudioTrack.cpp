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


ci::audio::DeviceRef AudioTrack::getOutputDevice() {
	static ci::audio::DeviceRef device = nullptr;

	if ( !device ) {
		auto ctx = audio::Context::master();
		auto dm = ctx->deviceManager();
		auto ni = dm->findDeviceByKey( "alsa_output.usb-Native_Instruments_Komplete_Audio_6_139D1FA3-00.analog-surround-21" );
		if ( ni ) {
			cout << "found NI" << endl;
			device = ni;
		} else {
			device = dm->getDefaultOutput();
			cout << "reverting to default audio device" << endl;
		}
	}

	return device;
}

AudioTrack::AudioTrack(): mDisplaySize( 1440, 100 ) {

}

AudioTrackRef AudioTrack::create( const ci::fs::path &path, size_t channel, int height ) {

	auto output = std::make_shared<AudioTrack>();



	auto sourceFile = audio::load( loadFile( path ) );
	output->mBuffer = sourceFile->loadBuffer();
	output->mCurrentBuffer = sourceFile->loadBuffer();

	auto ctx = audio::Context::master();

	auto device = AudioTrack::getOutputDevice();
	cout << "audio sample rate is: " << device->getSampleRate() << endl;

	static bool created = false;
	if ( !created ) {
		auto format = audio::Node::Format();
		format.setChannels( 2 );
		auto outputNode = ctx->createOutputDeviceNode( device, format );
		ctx->disable();
		ctx->setOutput( outputNode );
		ctx->enable();
		created = true;
	}

	output->mBufferPlayer = ctx->makeNode( new audio::BufferPlayerNode( output->mCurrentBuffer ) );
	output->mBufferPlayer->stop();
	output->mGain = ctx->makeNode( new audio::GainNode( 0.99f ) );
	output->mChannelRouter = ctx->makeNode( new audio::ChannelRouterNode( audio::Node::Format().channels( ctx->getOutput()->getNumChannels() ) ) );

	output->mBufferPlayer >> output->mGain >> output->mChannelRouter->route( 0, channel ) >>  ctx->getOutput(); //->getOutputs()[channel];
	output->mDisplaySize.y = height;
	output->generateTexture();
	output->mBufferPlayer->stop();
	output->mBufferPlayer->disable();

	output->mAllowsMute = false;
	output->mOriginalDuration = output->mBufferPlayer->getNumSeconds();

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
	return mOriginalDuration; 
}

int AudioTrack::getHeight() const {
	return mDisplaySize.y;
}

void AudioTrack::draw( int width ) const {
	gl::draw( mTexture, Rectf( 0, 0, width, mTexture->getHeight() ) );
}

void AudioTrack::generateTexture() {

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
		const auto N = mBuffer->getNumFrames();
		float xStep = mDisplaySize.x / static_cast<float>( N );

		float *data = mBuffer->getChannel( 0 );
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

void AudioTrack::setSpeed( float r ) {
	const   float *data = mBuffer->getChannel( 0 );
	size_t len = mBuffer->getNumFrames();
	size_t newLen = static_cast<size_t>( len / r );

	mCurrentBuffer = std::shared_ptr<audio::Buffer>( new audio::Buffer( newLen, 1 ) );
	float *newData = mCurrentBuffer->getData();
	size_t index;
	for ( size_t i = 0; i < newLen; i++ ) {
		index = static_cast<size_t>( math<float>::floor( i * r ) );
		newData[i] = data[index];
	}

	mBufferPlayer->setBuffer( mCurrentBuffer );

	cout << "setting speed to " << r << endl;
}





