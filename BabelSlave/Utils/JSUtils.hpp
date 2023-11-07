#pragma once
#include <string>
#include <AppCore/JSHelpers.h>
#include <JavaScriptCore/JSRetainPtr.h>

namespace Babel
{
	std::string ascii_to_utf8(const std::string& ascii_string);
	std::string utf8_to_ascii(const std::string& utf8_string);

	void SetObjectString(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, const char* value);
	
	template<typename T>
	void SetObjectNumber(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, T& value)
	{
		JSRetainPtr<JSStringRef> paramStr =
			adopt(JSStringCreateWithUTF8CString(paramName));
		auto jparam = JSValueMakeString(ctx, paramStr.get());
		auto jvalue = JSValueMakeNumber(ctx, value);
		JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
	}

	void SetObjectBoolean(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, bool value);
	void SetChildObject(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, JSObjectRef& value);
	void SetChildObject(JSContextRef& ctx, JSObjectRef& objectRef,const std::string& paramName, JSObjectRef& value);
}