#include "engine/actor.h"
#include "engine/caaf.h"
#include <generator>
#include <string>

#define STORAGE_TITLE_ROOT "models"

using namespace std;

namespace engine
{
namespace io
{

/*
 * Loads an actor from the title storage and any required dependencies.
 * Dependencies are cached so that they are not read more than once.
 * Returns false if actor was not found or could not be opened.
 */
bool loadActor(string name);

#ifdef CAAF_ENABLE_DEBUG_TOOLS
/*
 * Writes an actor back to the title storage.
 */
void writeActor(string name);

/*
 * Creates a new empty actor.
 */
void createActor(string name);

generator<actor::actor> getActors();

#endif

} // namespace io
} // namespace engine
