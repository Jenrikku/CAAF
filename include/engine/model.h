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
