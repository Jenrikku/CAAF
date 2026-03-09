#include "caaf.h"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <string>

using namespace std;

namespace engine
{
namespace model
{

class mesh
{
	SDL_GPUDevice *device;

  public:
	SDL_GPUBuffer *vtxBuf;
	SDL_GPUBuffer *idxBuf;

	uint32_t vtxOffsCnt;
	uint32_t *vtxOffsets;

#ifdef CAAF_ENABLE_DEBUG_TOOLS
	uint32_t vtxSize;
	uint32_t idxSize;
	uint8_t *vtxData;
	uint8_t *idxData;
#endif

	~mesh();
};

class model
{
	SDL_GPUDevice *device;

  public:
	string name;
	model *dependsOn;
	bool isDependency;

	uint32_t meshCnt;
	mesh *meshes;
	SDL_GPUGraphicsPipeline **pipelines;

	~model();
};

} // namespace model
} // namespace engine
