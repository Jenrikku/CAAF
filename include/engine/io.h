#include "engine/caaf.h"
#include "engine/model.h"
#include <generator>
#include <string>

#define STORAGE_CAAF_ROOT "models"

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
bool loadModel(string name);

#ifdef CAAF_ENABLE_DEBUG_TOOLS
/*
 * Reads a model from a path.
 */
bool readModel(string path);

/*
 * Writes an model to the desired path.
 */
bool writeModel(string name, string path);

/*
 * Creates a new empty model.
 */
void createModel(string name);

generator<model::model> getModels();

#endif

} // namespace io
} // namespace engine
