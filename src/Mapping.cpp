//
// Created by whg on 23/04/18.
//

#include "Mapping.hpp"

#include "cinder/app/App.h"
#include "cinder/Json.h"
#include "cinder/Log.h"
#include "cinder/gl/gl.h"

#include "Output.hpp"

#include "CinderImGui.h"

using namespace ci;

Mapping::Mapping( const ci::fs::path &path ): mMaxDmxChannel( 0 ), mNeedsToRedraw( true ), mTextureMag( 30 ), mDrawLabels( false ) {

	ci::JsonTree jsonFile( ci::loadFile( path ) );
	for ( const ci::JsonTree &json : jsonFile ) {
		int x = json.getValueForKey<int>( "x" );
		int y = json.getValueForKey<int>( "y" );
		auto name = json.getValueForKey<std::string>( "name" );
		int addr = 0;
		if ( json.hasChild( "addr" ) ) {
			addr = json.getValueForKey<int>( "addr" );
		} else {
			CI_LOG_W( "no addr for " << name );
		}

		mMap.emplace( glm::ivec2( x, y ), Point{ name, addr } );

		mMaxDmxChannel = std::max( mMaxDmxChannel, static_cast<size_t>( addr ) );
	}

	Output::get()->getDataSignal().connect( [this]() { this->mNeedsToRedraw = true; } );

}

std::vector<uint8_t> Mapping::dmxFromChannel( const ci::Channel &channel ) const {
	std::vector<uint8_t> output( mMaxDmxChannel, 0 );

	for ( const auto &pair : mMap ) {
		const glm::ivec2 &p = pair.first;
		int value = channel.getValue( p );
		output[pair.second.dmxAddr] = value;
	}
	return output;
}

void Mapping::draw() {
	if ( !mTexture || mNeedsToRedraw ) {
		generateTexture();
	}
	gl::draw( mTexture );
}

void Mapping::drawUi() {
	ui::ScopedWindow window( "Mapping" );
	if ( ui::Checkbox( "Draw labels ", &mDrawLabels ) ) {
		mNeedsToRedraw = true;
	}
	if ( mCurrentPoint ) {
		ui::LabelText( "Name", mCurrentPoint->name.c_str() );
		int addr = mCurrentPoint->dmxAddr + 1;
		ui::DragInt( "Address", &addr , 0.1f, 0, 500 );
	}
}

void Mapping::generateTexture() {

	ivec2 maxs( 0 );
	for ( const auto &pair : mMap ) {
		maxs.x = std::max( pair.first.x, maxs.x );
		maxs.y = std::max( pair.first.y, maxs.y );
	}
	maxs = ( maxs + ivec2( 1 ) ) * mTextureMag;

	gl::FboRef fbo = gl::Fbo::create( maxs.x, maxs.y );

	{
		gl::enableAlphaBlending();
		gl::ScopedFramebuffer fbScp( fbo );
		gl::clear( ColorA( 0, 0, 0, 0 ) );

		gl::ScopedViewport scpVp( ivec2( 0 ), fbo->getSize() );
		gl::ScopedMatrices m;
		gl::setMatricesWindow( fbo->getSize() );

		gl::translate( vec2( mTextureMag * 0.5f ) );

		float circleRadius = mTextureMag * 0.3f;
		auto &dmxData = Output::get()->getValueData();

		for ( const auto &pair : mMap ) {
			auto g = dmxData[pair.second.dmxAddr];
			gl::ScopedColor c( g, g, g );
			glm::vec2 pos = vec2( pair.first ) * static_cast<float>( mTextureMag );
			gl::drawSolidCircle( pos,  circleRadius );
			if ( mDrawLabels ) {
				gl::drawString( pair.second.name, pos );
			}
		}
	}

	auto surface = fbo->readPixels8u( Area( 0, 0, fbo->getWidth(), fbo->getHeight() ) );
	mTexture = gl::Texture::create( surface );

	mNeedsToRedraw = false;

}

void Mapping::mouseDown( ivec2 p, bool dryRun ) {
	p = ( p / mTextureMag );

	if ( !dryRun ) {
		try {
			mCurrentPoint = std::shared_ptr<Point>( &mMap.at( p ), [=](Point *p) {} );
//			std::cout << point.name << ", " << point.dmxAddr << std::endl;
			Output::get()->setValue( mCurrentPoint->dmxAddr, 255 );
		} catch( std::out_of_range &e ) {}
	}
}

void Mapping::mouseUp( ivec2 p ) {
	if ( mCurrentPoint != nullptr ) {
		Output::get()->setValue( mCurrentPoint->dmxAddr, 0  );
		mCurrentPoint = nullptr;
	}
}
