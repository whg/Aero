//
// Created by whg on 23/04/18.
//

#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#define DEFAULT_FRAMERATE 25.f

struct TransportObject {
	TransportObject(): mMuteUntilFrame( 0 ), mSpeed( 1.0f ), mAllowsMute( true ), mAllowsSpeedChange( true ), mAllowsNegativeCue( false ) {}
	virtual ~TransportObject() = default;

	virtual void setName( std::string name ) { mName = name; }
	virtual std::string getName() const { return mName; }

	virtual void setPlayhead( float t ) {}
	virtual void play() {}
	virtual void stop() {}
	virtual void setSpeed( float r ) { mSpeed = r; }
	virtual void setAllowSpeedChange( bool b ) { mAllowsSpeedChange = b; }
	virtual bool getAllowSpeedChange() const { return mAllowsSpeedChange; }

	virtual bool allowsNegativeCue() { return mAllowsNegativeCue; }
	virtual void setAllowsNegativeCue( bool b) { mAllowsNegativeCue = b; }

	virtual float getDuration() const {}
	virtual int getHeight() const {}

	virtual void draw( int width ) const {}

	virtual bool getAllowsMute() const { return mAllowsMute; }
	virtual void setAllowsMute( bool b ) { mAllowsMute = b; }
	virtual int getMuteUntilFrame() const { return mMuteUntilFrame; }
	virtual float getMuteUntil() const { return mMuteUntilFrame / DEFAULT_FRAMERATE; }
	virtual void setMuteUntilFrame( int frame ) { mMuteUntilFrame = frame; }
	virtual void updateMute( int frame ) {}

	virtual bool isFrameBased() const { return false; }
	virtual size_t getNumFrames() const { return 0; }

	virtual void writeData( size_t frameNum ) const {}

protected:
	bool mAllowsMute;
	int mMuteUntilFrame;
	std::string mName;
	float mSpeed;
	bool mAllowsSpeedChange;
	bool mAllowsNegativeCue;
};

using TransportObjectRef = std::shared_ptr<TransportObject>;

class Transport {
public:
	Transport();

	void add( TransportObjectRef object );
	void remove( std::string name );
	void replace( std::string name, TransportObjectRef object );

	void draw();
	void drawUi();

	void play();
	void pause();
	void stop();

	void setDisplayWidth( int width ) { mDisplaySize.x = width; }
	void setFrameRate( float frameRate ) { mFrameRate = frameRate; }


	bool isPlaying() const { return mPlaying.load(); }
	int getHeight() const;

protected:
	std::atomic<int> mFrameNumber;
	std::atomic<float> mPlayhead;
	float mFrameRate;
	int mEndFrame;
	glm::ivec2 mDisplaySize;
	float mDuration;
	std::vector<TransportObjectRef> mObjects;

	int mCueFrame, mStopFrame;
	std::atomic<bool> mStarted;

	std::thread mPlayThread;
	std::atomic<bool> mPlaying;
	std::mutex mMutex;

	using scoped_lock = std::lock_guard<std::mutex>;

	void update();

protected:
	float timeToScreen( float t ) { return t / mDuration * mDisplaySize.x; }
	float screenToTime( int x ) { return static_cast<float>( x ) / mDisplaySize.x * mDuration; }
public:
	void mouseDown( glm::ivec2 p );
};

