#pragma once

#include <cstdint>
#include <string>

#define CAAF_ENABLE_DEBUG_TOOLS
#define CAAF_LZMA_LEVEL 5
#define CAAF_DECOMP_MEMORY_MAX 68157440 // 65M

#define CAAF_HEADER_MAGIC "CAAF"
#define CAAF_VERSION 0

#define CAAF_SECTION_LIST_POS 0x10

// EnFlags definitions:

#define CAAF_GFXP_ENDBIAS 0b00000001
#define CAAF_GFXP_ENDCLIP 0b00000010
#define CAAF_GFXP_ENDTEST 0b00000100
#define CAAF_GFXP_ENDWRT 0b00001000
#define CAAF_GFXP_ENDSTCL 0b00010000
#define CAAF_GFXP_ENA2COV 0b00100000
#define CAAF_GFXP_ENMASK 0b00100000

#define CAAF_CTB_ENBLEND 0b00000001
#define CAAF_CTB_ENMASK 0b00000010

#define CAAF_SAMP_ENANIS 0b00000001
#define CAAF_SAMP_ENCOMP 0b00000010

using namespace std;

namespace engine
{
namespace caaf
{

enum section { unknown, STRT, MESH, GFXP, TEXD, SAMP };

typedef uint16_t index;

// Section header
typedef struct secHeader {
	char magic[4];
	uint32_t count;
} secHeader;

// Subsection header
typedef struct subHeader {
	uint16_t count;
	uint16_t size; // per element
} subHeader;

// CAAF file header
typedef struct header {
	char magic[4];
	uint8_t version;
	uint8_t isDep;
	uint16_t sectCnt;
	uint16_t nameIdx;
	uint16_t depIdx;
} header;

// Mesh entry
typedef struct mesh {
	uint32_t vbdPtr;
	uint32_t vtxSize;
	uint32_t idxSize;
	uint32_t meshPtr;
} mesh;

// Vertex Buffer Data entry
typedef struct vtxBufData {
	uint32_t start;
} vtxBufData;

// Graphics Pipeline entry
typedef struct gfxPip {
	uint16_t vertNameIdx;
	uint16_t fragNameIdx;
	uint8_t primType;
	uint8_t fillMod;
	uint8_t cullMod;
	uint8_t frontFace;
	uint8_t msaaCnt;
	uint8_t compOp;
	uint8_t backOp;
	uint8_t frontOp;
	uint8_t cmpMask;
	uint8_t wrtMask;
	uint16_t enFlags;
	float depthBiasConst;
	float depthBiasClamp;
	float depthBiasSlope;
	uint32_t props;
	uint32_t vbdPtr;
	uint32_t vaPtr;
	uint32_t ctbPtr;
	uint32_t tsbPtr;
} gfxPip;

// Vertex Buffer Description entry
typedef struct vtxBufDesc {
	uint32_t pitch;
	uint32_t instStp;
} vtxBufDesc;

// Vertex Attribute entry
typedef struct vtxAttr {
	uint16_t loc;
	uint16_t format;
	uint32_t slot;
	uint32_t offset;
} vtxAttr;

// Vertex Attribute entry
typedef struct colTargBlend {
	uint8_t srcCol;
	uint8_t dstCol;
	uint8_t colBlendOp;
	uint8_t srcAlpha;
	uint8_t dstAlpha;
	uint8_t alphaBlendOp;
	uint8_t rgbaMask;
	uint8_t enFlags;
} colTargBlend;

// Texture Sampler Binding entry
typedef struct textSampBind {
	uint32_t slot;
	uint16_t textIdx;
	uint16_t sampIdx;
	uint8_t shStage;
} textSampBind;

// Texture entry
typedef struct texture {
	uint8_t type;
	uint8_t format;
	uint16_t mipLvls;
	uint32_t width;
	uint32_t height;
	uint32_t depth;
	uint32_t props;
	uint32_t dataPtr;
} texture;

// Sampler entry
typedef struct sampler {
	uint8_t minFilt;
	uint8_t magFilt;
	uint8_t mapMode;
	uint8_t addrModeU;
	uint8_t addrModeV;
	uint8_t addrModeW;
	uint8_t compOp;
	uint8_t enFlags;
	float mipLODBias;
	float maxAnis;
	float minLOD;
	float maxLOD;
	uint32_t props;
} sampler;

// Returns a pointer to the start of the section, meant to be used with other functions.
uint8_t *getSectionStart(uint8_t *caaf, uint16_t idx);

// Identifies the type of section from the pointer.
section identifySection(uint8_t *secStart);

// Gets the amount of entries inside the section.
uint16_t getSecEntryCnt(uint8_t *secStart);

// Returns a pointer to the start of a section's entry by index.
uint8_t *getSecEntryPtr(uint8_t *secStart, uint16_t idx);

// Gets the amount of entries inside the subsection.
uint16_t getSubEntryCnt(uint8_t *subStart);

// Returns a pointer to the start of a subsection's entry by index.
uint8_t *getSubEntryPtr(uint8_t *subStart, uint16_t idx);

// Gets a string from the string table section by index.
string getString(uint8_t *strSec, uint16_t idx, uint16_t limit = UINT16_MAX);

} // namespace caaf
} // namespace engine
