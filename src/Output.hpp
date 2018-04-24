//
// Created by whg on 23/04/18.
//

#pragma once

#include <memory>
#include <functional>

#include "cinder/Signals.h"

#include "whelpersg/serial.hpp"

#define NUM_DMX_BUFFERS 2

using OutputRef = std::shared_ptr<class Output>;

class Output {
public:
	using DataSignal_t = cinder::signals::Signal<void()>;

public:
	~Output();
	static OutputRef get();

	void setValue( size_t index, uint8_t value );
	void setValues( const std::vector<uint8_t> &values );
	void setValues( uint8_t value );

	void drawUi();

	DataSignal_t& getDataSignal() { return mDataSignal; }
	const std::vector<uint8_t>& getValueData() const { return mValueData; }

	void writeData();

protected:
	std::vector<uint8_t> mDmxData, mValueData;
	whg::Serial mSerial;

	DataSignal_t mDataSignal;

	int mValueThreshold;

private:
	Output();
};

