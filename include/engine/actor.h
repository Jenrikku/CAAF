#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_gpu.h>
#include <string>

using namespace std;

namespace engine
{
namespace actor
{

class mesh
{
};

class actor
{
	string name;
	uint32_t meshcnt;
	mesh *meshes;
	SDL_GPUGraphicsPipeline *pipelines;
};

typedef actor dependency;

} // namespace actor
} // namespace engine
