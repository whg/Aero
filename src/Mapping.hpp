//
// Created by whg on 22/04/18.
//

#pragma  once

#include <unordered_map>
#include <vector>
#include <cinder/gl/Texture.h>

#include "cinder/CinderGlm.h"
#include "cinder/Filesystem.h"
#include "cinder/Channel.h"

struct HashPred {
	size_t operator()( const glm::ivec2& v ) const {
		return std::hash<int>()( v.x ) ^ std::hash<int>()( v.y );
	}

	bool operator()( const glm::ivec2& a, const glm::ivec2& b )const {
		return a.x == b.x && a.y == b.y;
	}
};

class Mapping {
public:
	struct Point {
		std::string name;
		int dmxAddr;
	};

public:
	Mapping( const ci::fs::path &path );

	std::vector<uint8_t> dmxFromChannel( const ci::Channel &channel ) const;

	void draw();
	void drawUi();

	size_t size() const { return mMap.size(); }
	size_t getMaxDmxChannel() const { return mMaxDmxChannel; }

protected:
	using map_t = std::unordered_map<glm::ivec2, Point, HashPred, HashPred>;

protected:
	map_t mMap;
	size_t mMaxDmxChannel;

protected:
	ci::gl::TextureRef mTexture;
	int mTextureMag;
	bool mNeedsToRedraw;
	bool mDrawLabels;

	void generateTexture();
	void setNeedsToRedraw( bool b ) { mNeedsToRedraw = b; }

public:
	void mouseDown( glm::ivec2 p, bool dryRun=false );
	void mouseUp( glm::ivec2 p );
	std::shared_ptr<Point> mCurrentPoint;
};