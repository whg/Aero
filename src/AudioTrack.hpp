//
// Created by whg on 23/04/18.
//

#pragma once

#include "cinder/audio/audio.h"
#include "cinder/gl/gl.h"

#include "Transport.hpp"

using AudioTrackRef = std::shared_ptr<class AudioTrack>;

class AudioTrack : public TransportObject {
public:
	AudioTrack();

	static AudioTrackRef create( const ci::fs::path &path, size_t channel, int height=100 );
	static ci::audio::DeviceRef getOutputDevice();

	void play() override;
	void stop() override;

	void setPlayhead( float t ) override;
	void setSpeed( float r ) override;

	float getDuration() const override;
	int getHeight() const override;
	void draw( int width ) const override;

	void updateMute( int frame) override;

	void setLeadinTime( float time ) { mLeadinTime = time; }

	void generateTexture();

protected:
	ci::audio::BufferRef mBuffer, mCurrentBuffer;
	ci::audio::GainNodeRef mGain;
	ci::audio::BufferPlayerNodeRef mBufferPlayer;
	ci::audio::ChannelRouterNodeRef mChannelRouter;

protected:
	glm::ivec2 mDisplaySize;
	ci::gl::TextureRef mTexture;

	float mOriginalDuration;
	float mLeadinTime;
};

