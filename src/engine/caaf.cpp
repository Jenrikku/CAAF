#include "engine/caaf.h"
#include <cstdint>

namespace engine
{
namespace caaf
{

uint8_t *getSectionStart(uint8_t *caaf, uint16_t idx)
{
	uint32_t secOffset = *(uint32_t *)(caaf + CAAF_SECTION_LIST_POS + (idx << 2));
	return caaf + secOffset;
}

section identifySection(uint8_t *secStart)
{
	secHeader header = *(secHeader *)secStart;
	string str(header.magic, sizeof(header.magic));

	if (str == "STRT") return STRT;
	if (str == "GFXP") return GFXP;
	if (str == "TEXD") return TEXD;
	if (str == "SAMP") return SAMP;

	return unknown;
}

uint8_t *getEntryPtr(uint8_t *secStart, uint16_t idx)
{
	uint8_t *ptrPos = secStart + sizeof(secHeader) + (idx << 2);
	uint32_t offset = *(uint32_t *)ptrPos;
	return ptrPos + offset;
}

string getString(uint8_t *strSec, uint16_t idx)
{
	const char *str = (char *)getEntryPtr(strSec, idx);
	return string(str);
}

} // namespace caaf
} // namespace engine