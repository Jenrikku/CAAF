#include "engine/io.h"
#include "engine/caaf.h"
#include "engine/lzma.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_storage.h>
#include <SDL3/SDL_timer.h>
#include <cstdint>
#include <iostream>
#include <memory>

namespace engine
{
namespace io
{

bool loadActor(string name)
{
	string path = name + ".caaf.xz";
	SDL_Storage *storage = SDL_OpenTitleStorage(STORAGE_TITLE_ROOT, 0);

	if (!storage) {
		cerr << SDL_GetError();
		return false;
	}

	while (!SDL_StorageReady(storage))
		SDL_Delay(1);

	ulong size;
	if (!SDL_GetStorageFileSize(storage, path.c_str(), &size)) return false;

	uint8_t *xz = new uint8_t[size];

	if (!SDL_ReadStorageFile(storage, path.c_str(), xz, size)) {
		delete[] xz;
		return false;
	}

	// Decompress XZ:
	uint8_t *caaf = lzma::decompress(xz, &size);
	delete[] xz;

	if (caaf == nullptr) return false;

	unique_ptr<uint8_t> caafptr(caaf); // For safe deletion once out of scope.

	caaf::header header = *(caaf::header *)caaf;
	if (string(header.magic) != CAAF_HEADER_MAGIC || header.version != CAAF_VERSION) return false;

	// Code here

	return true;
}

} // namespace io
} // namespace engine