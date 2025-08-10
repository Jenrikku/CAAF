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
};

class model
{
  public:
	string name;
	model *dependsOn;
	bool isDependency;

	uint32_t meshCnt;
	mesh *meshes;
	SDL_GPUGraphicsPipeline *pipelines;
};

} // namespace model
} // namespace engine
