#include "helpers.h"
#include <cstring>

std::vector<std::shared_ptr<char[]>> ClayStringHelpers::vStringBuffers;

ClayStringHelpers::ClayStringHelpers(const std::string& tString) { Allocate(tString.c_str(), tString.size()); }
ClayStringHelpers::ClayStringHelpers(const char* tString) { Allocate(tString, std::strlen(tString)); }
void ClayStringHelpers::ClearBuffers() { vStringBuffers.clear(); }

void ClayStringHelpers::Allocate(const char* tString, size_t iLength) {
	std::shared_ptr<char[]> pTempBuffer = std::make_shared<char[]>(iLength + 1);
	std::memcpy(pTempBuffer.get(), tString, iLength);
	pTempBuffer[iLength] = '\0';
	vStringBuffers.push_back(pTempBuffer);

	sReturnString = { false, (int32_t)iLength, pTempBuffer.get() };
}