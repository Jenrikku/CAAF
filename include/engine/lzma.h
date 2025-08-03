#include "engine/caaf.h"
#include <cstddef>
#include <cstdint>
#include <lzma.h>
#include <string>

namespace engine
{
namespace lzma
{

/*
 * The compressed filesize needs to be passed through size.
 * Returns a pointer to decompressed data or nullptr if error.
 * Puts size to the final size of the decompressed data.
 */
uint8_t *decompress(const uint8_t *data, size_t *size);

#ifdef CAAF_ENABLE_DEBUG_TOOLS
/*
 * Compresses a file from memory to a file.
 * Returns true on success, false otherwise.
 */
bool compress(const uint8_t *data, size_t size, string fileout);
#endif

} // namespace lzma
} // namespace engine