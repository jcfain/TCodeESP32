
#pragma once

// constants.h first so the firmware's TCODE_DEVICE_INFO is defined before
// TCode.h's "Generic TCode Device" fallback (avoids a macro redefinition).
// Note: lib/TCode/TCode.cpp is compiled on its own and still uses the fallback
// for its D0 response unless TCODE_DEVICE_INFO is supplied as a build flag.
#include "constants.h"
#include "TCode.h"
#include "logging/TagHandler.h"
#include "TCodeBase.h"

class TCode0_4 : public TCodeBase, public TCode
{
public:
	TCode0_4()
	{
		// The TCode library reports D0/D1/D2/DSTOP (and $/# passthrough)
		// through a plain function pointer. Route it through
		// TCodeBase::sendMessage so responses reach the message callback when
		// one has been wired, or the default serial callback otherwise (same
		// behaviour as TCode0_3). Only one TCode0_4 instance exists.
		s_instance = this;
		TCode::setTCodeCallback(&TCode0_4::forwardResponse);
	}

	// Setup function
	void setup(const char *firmware) override
	{
		firmwareID = firmware;
	}

	void read(byte inByte) override
	{
		TCode::byteInput(inByte);
	}

	void read(const char* in) override
	{
		TCode::stringInput(in);
	}

	void setMessageCallback(TCodeCallback f) override
	{
		// Library responses are forwarded via forwardResponse -> sendMessage,
		// which uses this callback.
		TCodeBase::setMessageCallback(f);
	}

private:
	static void forwardResponse(const char *text)
	{
		if (s_instance && text)
			s_instance->sendMessage(text);
	}

	static constexpr Tags::tag_t _TAG = Tags::TCode;
	const char *firmwareID;
	inline static TCode0_4 *s_instance = nullptr;
};
