//
// Created by whg on 22/04/18.
//

#pragma once

#include <cinder/gl/Texture.h>
#include "Mapping.hpp"
#include "Transport.hpp"

using FrameSequenceRef = std::shared_ptr<class FrameSequence>;

class FrameSequence : public TransportObject {
public:
	using DmxSequence = std::vector<uint8_t>;

public:
	static FrameSequenceRef create( const ci::fs::path &imageFolder, const Mapping &mapping, float frameRate=25.f );
	void setup( const ci::fs::path &imageFolder, const Mapping &mapping, float frameRate=25.f );
	float getDuration() const override;
	int getHeight() const override;
	void draw( int width ) const override;

	bool isFrameBased() const override { return true; }
	size_t getNumFrames() const { return mDmxSequences.size(); }

	void writeData( size_t frameNum ) const override;

protected:
	std::vector<DmxSequence> mDmxSequences;
	float mFrameRate;

protected:
	int mDisplayHeight;
	ci::gl::TextureRef mTexture;
	ci::gl::TextureRef generateTexture();
};

