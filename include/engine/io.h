#include "engine/caaf.h"
#include "engine/model.h"
#include <generator>
#include <string>

#define STORAGE_CAAF_ROOT "models"
#define STORAGE_CSAF_ROOT "shaders"

using namespace std;

namespace engine
{
namespace io
{

/*
 * Loads a model from the title storage and any required dependencies.
 * Models are cached so that they are not read more than once.
 * Returns false if a model was not found or could not be opened.
 */
bool loadModel(string name, SDL_GPUDevice *device, SDL_GPUCopyPass *pass);

/*
 * Loads a shader from the title storage.
 * Shaders are cached so that they are not read more than once.
 * Returns false if a shader was not found or could not be opened.
 */
bool loadShader(string name, SDL_GPUDevice *device, SDL_GPUCopyPass *pass);

/*
 * Clears all loaded models from memory.
 */
void clearModels();

/*
 * Clears all loaded shaders from memory.
 */
void clearShaders();

/*
 * Returns a shader format compatible with the current platform from the ones passed.
 */
SDL_GPUShaderFormat resolvePlatformShaderFormat(SDL_GPUShaderFormat formats, SDL_GPUDevice *device);

#ifdef CAAF_ENABLE_DEBUG_TOOLS
/*
 * Reads a model from a path.
 */
bool readModel(string path, SDL_GPUDevice *device, SDL_GPUCopyPass *pass);

/*
 * Writes an model to the desired path.
 */
bool writeModel(string name, string path);

/*
 * Creates a new empty model.
 */
void createModel(string name);

/*
 * Reads a shader from a path.
 */
bool loadShader(string path, SDL_GPUDevice *device, SDL_GPUCopyPass *pass);

generator<model::model> getModels();

#endif

} // namespace io
} // namespace engine
