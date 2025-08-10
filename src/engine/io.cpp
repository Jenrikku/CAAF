#include "engine/io.h"
#include "engine/caaf.h"
#include "engine/lzma.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_storage.h>
#include <SDL3/SDL_timer.h>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

namespace engine
{
namespace io
{

static unordered_map<string, model::model *> loadedModels;

// internal method
model::model *loadModel(uint8_t *caaf, size_t size, string *deps)
{
	caaf::header header = *(caaf::header *)caaf;
	if (string(header.magic) != CAAF_HEADER_MAGIC || header.version != CAAF_VERSION) return nullptr;

	model::model *modl = new model::model();

	uint8_t *strSec = caaf::getSectionStart(caaf, 0);

	modl->name = caaf::getString(strSec, header.nameIdx);
	*deps = caaf::getString(strSec, header.depIdx);

	for (int i = 1; i < header.sectCnt; i++) {
		uint8_t *secStart = caaf::getSectionStart(caaf, i);
		caaf::section secType = caaf::identifySection(secStart);
	}

	return modl;
}

bool loadModel(string name)
{
	if (loadedModels.contains(name)) return true;

	string path = name + ".caaf.xz";
	SDL_Storage *storage = SDL_OpenTitleStorage(STORAGE_CAAF_ROOT, 0);

	if (!storage) {
		cerr << SDL_GetError();
		return false;
	}

	while (!SDL_StorageReady(storage))
		SDL_Delay(1);

	size_t size;
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

	// Load model:
	string dependency;
	model::model *modl = loadModel(caaf, size, &dependency);

	delete[] caaf;

	loadedModels.insert({modl->name, modl});

	if (dependency.empty()) return modl != nullptr; // No dependency or error.

	if (!loadModel(dependency)) return false;

	modl->dependsOn = loadedModels.extract(dependency).mapped();
	return true;
}

#ifdef CAAF_ENABLE_DEBUG_TOOLS

bool readModel(string path)
{
	size_t size;
	void *xz = SDL_LoadFile(path.c_str(), &size);

	// Decompress XZ:
	uint8_t *caaf = lzma::decompress((uint8_t *)xz, &size);
	SDL_free(xz);

	if (caaf == nullptr) return false;

	// Load model:
	string dependency;
	model::model *modl = loadModel(caaf, size, &dependency);

	delete[] caaf;

	loadedModels.insert({modl->name, modl});

	if (dependency.empty()) return modl != nullptr; // No dependency or error.

	if (!loadModel(dependency)) return false;

	modl->dependsOn = loadedModels.extract(dependency).mapped();
	return true;
}

#endif

} // namespace io
} // namespace engine