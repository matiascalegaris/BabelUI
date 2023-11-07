#include "JSUtils.hpp"
#include <Windows.h>

namespace Babel
{
	std::string ascii_to_utf8(const std::string& ascii_string)
	{
		int num_wchars = MultiByteToWideChar(CP_ACP, 0, ascii_string.c_str(), -1, NULL, 0);
		std::unique_ptr<wchar_t[]> wide_string(new wchar_t[num_wchars]);
		MultiByteToWideChar(CP_ACP, 0, ascii_string.c_str(), -1, wide_string.get(), num_wchars);

		int num_chars = WideCharToMultiByte(CP_UTF8, 0, wide_string.get(), -1, NULL, 0, NULL, NULL);
		std::unique_ptr<char[]> utf8_string(new char[num_chars]);
		WideCharToMultiByte(CP_UTF8, 0, wide_string.get(), -1, utf8_string.get(), num_chars, NULL, NULL);

		return std::string(utf8_string.get());
	}

	std::string utf8_to_ascii(const std::string& utf8_string)
	{
		int num_wchars = MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, NULL, 0);
		std::unique_ptr<wchar_t[]> wide_string(new wchar_t[num_wchars]);
		MultiByteToWideChar(CP_UTF8, 0, utf8_string.c_str(), -1, wide_string.get(), num_wchars);

		int num_chars = WideCharToMultiByte(CP_ACP, 0, wide_string.get(), -1, NULL, 0, NULL, NULL);
		std::unique_ptr<char[]> ascii_string(new char[num_chars]);
		WideCharToMultiByte(CP_ACP, 0, wide_string.get(), -1, ascii_string.get(), num_chars, NULL, NULL);

		return std::string(ascii_string.get());
	}

	void SetObjectString(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, const char* value)
	{
		JSRetainPtr<JSStringRef> valueStr =
			adopt(JSStringCreateWithUTF8CString(value));
		JSRetainPtr<JSStringRef> paramStr =
			adopt(JSStringCreateWithUTF8CString(paramName));
		auto jparam = JSValueMakeString(ctx, paramStr.get());
		auto jvalue = JSValueMakeString(ctx, valueStr.get());
		JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
	}

	void SetObjectBoolean(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, bool value)
	{
		JSRetainPtr<JSStringRef> paramStr =
			adopt(JSStringCreateWithUTF8CString(paramName));
		auto jparam = JSValueMakeString(ctx, paramStr.get());
		auto jvalue = JSValueMakeBoolean(ctx, value);
		JSObjectSetProperty(ctx, objectRef, paramStr.get(), jvalue, 0, nullptr);
	}

	void SetChildObject(JSContextRef& ctx, JSObjectRef& objectRef, const char* paramName, JSObjectRef& value)
	{
		JSRetainPtr<JSStringRef> paramStr =
			adopt(JSStringCreateWithUTF8CString(paramName));
		auto jparam = JSValueMakeString(ctx, paramStr.get());
		JSObjectSetProperty(ctx, objectRef, paramStr.get(), value, 0, nullptr);
	}

	void SetChildObject(JSContextRef& ctx, JSObjectRef& objectRef, const std::string& paramName, JSObjectRef& value)
	{
		JSRetainPtr<JSStringRef> paramStr =
			adopt(JSStringCreateWithUTF8CString(paramName.c_str()));
		auto jparam = JSValueMakeString(ctx, paramStr.get());
		JSObjectSetProperty(ctx, objectRef, paramStr.get(), value, 0, nullptr);
	}

}