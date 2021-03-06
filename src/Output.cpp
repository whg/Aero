//
// Created by whg on 23/04/18.
//

#include "Output.hpp"

#include "CinderImGui.h"

#define DMXPRO_START_MSG 0x7E
#define DMXPRO_END_MSG	0xE7
#define DMXPRO_SEND_LABEL 6
#define DMXPRO_BAUD_RATE 57600
#define DMXPRO_DATA_SIZE 513
#define DMXPRO_PACKET_SIZE  518
#define DMXPRO_START_INDEX 5

using namespace std;

Output::Output(): mDmxData( DMXPRO_PACKET_SIZE, 0), mValueData( 500, 0 ), mValueThreshold( 128 ), mLive( true ) {

	mDmxData[0] = DMXPRO_START_MSG;
	mDmxData[1] = DMXPRO_SEND_LABEL;
	mDmxData[2] = (int)DMXPRO_DATA_SIZE & 0xff;
	mDmxData[3] = ((int)DMXPRO_DATA_SIZE >> 8) & 0xff;
	mDmxData[4] = 0;
	mDmxData[DMXPRO_PACKET_SIZE-1] = DMXPRO_END_MSG;

	mSerial.open( "ttyUSB0", DMXPRO_BAUD_RATE );

	getDataSignal().connect( std::bind( &Output::writeData, this ) );

}

Output::~Output() {
	mSerial.close();
}

OutputRef Output::get() {
	static OutputRef instance = nullptr;
	if ( !instance ) {
		instance = shared_ptr<Output>( new Output );
	}
	return instance;
}

void Output::drawUi() {
	ui::ScopedWindow window( "Output" );

	ui::LabelText( mSerial.isOpen() ? "Connected" : "-", "DMX status: " );
	ui::DragInt( "Threshold", &mValueThreshold, 0.1f, 0, 255 );
	if ( ui::Button( "All on" ) ) {
		setValues( 255 );
	}
	ui::SameLine();
	if ( ui::Button( "All off" ) ) {
		setValues( 0 );
	}

	ui::Checkbox( "Live", &mLive );
}

void Output::setValue( size_t index, uint8_t value ) {
//	mDmxData[DMXPRO_START_INDEX + index] = value;
	mValueData[index] = static_cast<uint8_t>( value > mValueThreshold ? 255 : 0 );

	mDataSignal.emit();
}

void Output::setValues( const std::vector<uint8_t> &values ) {
	std::copy( values.begin(), values.end(), mValueData.begin() );
	for ( uint8_t &v : mValueData ) {
		v = static_cast<uint8_t>( v > mValueThreshold ? 255 : 0 );
	}
	for (int i = 0; i < 5; i++) {
		mValueData[81+i] = 0;
	}
	mDataSignal.emit();
}

void Output::writeData() {
	std::copy( mValueData.begin(), mValueData.end(), mDmxData.begin() + DMXPRO_START_INDEX );
	if ( mLive ) {
		mSerial.write( mDmxData );
	}
}

void Output::setValues( uint8_t value ) {
	std::fill( mValueData.begin(), mValueData.end(), value );
	for (int i = 0; i < 5; i++) {
		mValueData[81+i] = 0;
	}
	mDataSignal.emit();
}




