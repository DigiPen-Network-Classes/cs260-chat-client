#pragma once
#include "clay.h"
#include <string>
#include <memory>
#include <vector>

class ClayStringHelpers {
public:
	ClayStringHelpers(const std::string& tString);
	ClayStringHelpers(const char* tString);
	~ClayStringHelpers() = default;

	operator Clay_String() const { return sReturnString; }
	static void ClearBuffers();

	Clay_String sReturnString;

private:
	static std::vector<std::shared_ptr<char[]>> vStringBuffers;
	void Allocate(const char* tString, size_t iLength);
};

inline ClayStringHelpers C(const char* tString) { return ClayStringHelpers(tString); }
inline ClayStringHelpers C(const std::string& tString) { return ClayStringHelpers(tString); }