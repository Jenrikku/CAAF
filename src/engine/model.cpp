#include "engine/model.h"
#include <SDL3/SDL_gpu.h>

namespace engine
{
namespace model
{

mesh::~mesh()
{
	SDL_ReleaseGPUBuffer(device, vtxBuf);
	SDL_ReleaseGPUBuffer(device, idxBuf);

	delete[] vtxOffsets;

#ifdef CAAF_ENABLE_DEBUG_TOOLS
	delete[] vtxData;
	delete[] idxData;
#endif
}

model::~model()
{
	for (uint32_t i = 0; i < meshCnt; i++)
		SDL_ReleaseGPUGraphicsPipeline(device, pipelines[i]);

	delete[] meshes;
	delete[] pipelines;
}

} // namespace model
} // namespace engine
