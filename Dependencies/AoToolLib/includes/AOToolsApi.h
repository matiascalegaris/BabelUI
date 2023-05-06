#pragma once
#include <string>
#include <vector>

std::vector<uint8_t> __cdecl GetTelemetry(const std::string& code);

std::string __cdecl CheckIdErrors(int telemetryId);

std::string __cdecl GetTelemetryCode(int telemetryId, const std::string& name);

std::string __cdecl GetTelemetryResult(std::vector<uint8_t>& data, int TelemetryId);