#pragma once

#include <filesystem>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace gs
{

	inline std::filesystem::path pathFromText(const std::string& value)
	{
#ifdef _WIN32
		auto decodeMultiByte = [&value](UINT codePage, DWORD flags, std::wstring& outWide) -> bool
		{
			if (value.empty())
			{
				outWide.clear();
				return true;
			}

			const int wideCount = MultiByteToWideChar(
				codePage,
				flags,
				value.data(),
				static_cast<int>(value.size()),
				nullptr,
				0);
			if (wideCount <= 0)
			{
				return false;
			}

			outWide.resize(static_cast<std::size_t>(wideCount));
			return MultiByteToWideChar(
				codePage,
				flags,
				value.data(),
				static_cast<int>(value.size()),
				outWide.data(),
				wideCount) == wideCount;
		};

		std::wstring wideValue;
		if (decodeMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, wideValue) ||
			decodeMultiByte(CP_ACP, 0, wideValue) ||
			decodeMultiByte(CP_OEMCP, 0, wideValue))
		{
			return std::filesystem::path(wideValue);
		}

		return std::filesystem::path(value.begin(), value.end());
#else
		return std::filesystem::u8path(value);
#endif
	}

	inline std::string pathToUtf8(const std::filesystem::path& value)
	{
#ifdef _WIN32
		const std::wstring wideValue = value.generic_wstring();
		if (wideValue.empty())
		{
			return {};
		}

		const int utf8Count = WideCharToMultiByte(
			CP_UTF8,
			0,
			wideValue.data(),
			static_cast<int>(wideValue.size()),
			nullptr,
			0,
			nullptr,
			nullptr);
		if (utf8Count <= 0)
		{
			return {};
		}

		std::string utf8Value(static_cast<std::size_t>(utf8Count), '\0');
		const int written = WideCharToMultiByte(
			CP_UTF8,
			0,
			wideValue.data(),
			static_cast<int>(wideValue.size()),
			utf8Value.data(),
			utf8Count,
			nullptr,
			nullptr);
		if (written != utf8Count)
		{
			return {};
		}

		return utf8Value;
#else
		return value.generic_string();
#endif
	}

} // namespace gs