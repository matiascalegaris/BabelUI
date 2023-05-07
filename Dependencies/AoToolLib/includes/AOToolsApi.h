#pragma once
#include <string>
#include <vector>

extern "C"
{
	int __cdecl CheckIdErrors(int telemetryId, char* outBuffer, int outLen);

	int __cdecl GetTelemetryCode(int telemetryId, const char* name, char* outBuffer, int outLen);

	int __cdecl GetTelemetryResult(const char* data, int dataSize, int telemetryId, char* outBuffer, int outLen);

	uint32_t __cdecl GetTelemetryData(const char* str, const uint8_t* data, uint32_t maxSize);
}