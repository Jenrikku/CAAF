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
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>

#define EXT_CAAF ".caaf.xz"
#define EXT_CSAF ".csaf.xz"

namespace engine
{
namespace io
{

static unordered_map<string, model::model *> loadedModels;
static unordered_map<string, SDL_GPUShader *> loadedShaders;

// internal method
uint8_t *loadCommon(const char *path, const char *root)
{
	SDL_Storage *storage = SDL_OpenTitleStorage(root, 0);

	if (!storage) {
		cerr << SDL_GetError() << endl;
		return nullptr;
	}

	while (!SDL_StorageReady(storage))
		SDL_Delay(1);

	size_t size;
	if (!SDL_GetStorageFileSize(storage, path, &size)) return nullptr;

	uint8_t *xz = new uint8_t[size];

	if (!SDL_ReadStorageFile(storage, path, xz, size)) {
		delete[] xz;
		return nullptr;
	}

	SDL_CloseStorage(storage);

	// Decompress XZ:
	uint8_t *res = lzma::decompress(xz, &size);
	delete[] xz;

	return res;
}

#ifdef CAAF_ENABLE_DEBUG_TOOLS

// internal method
uint8_t *readCommon(const char *path, const char *root)
{
	filesystem::path file = filesystem::path(root).append(path);

	size_t size;
	void *xz = SDL_LoadFile(file.c_str(), &size);

	// Decompress XZ:
	uint8_t *res = lzma::decompress((uint8_t *)xz, &size);
	SDL_free(xz);

	return res;
}

#endif

// internal method
model::model *loadModel(uint8_t *caaf, const char *root, uint8_t *(*depsFunc)(const char *, const char *),
						SDL_GPUDevice *device, SDL_GPUCopyPass *pass)
{
	caaf::header header = *(caaf::header *)caaf;

	// Possible errors: magic number does not match, version does not match, no sections (at least STRT is required)
	if (string(header.magic) != CAAF_HEADER_MAGIC || header.version != CAAF_VERSION || !header.sectCnt) return nullptr;

	model::model *modl = new model::model();

	uint8_t *strSec = caaf::getSectionStart(caaf, 0);

	if (caaf::identifySection(strSec) != caaf::section::STRT) {
		cerr << "Malformed CAAF: STRT was not the first section." << endl;
		return modl;
	}

	uint16_t strLimit = caaf::getSecEntryCnt(strSec) - 1;

	modl->name = caaf::getString(strSec, header.nameIdx, strLimit);
	string dependency = caaf::getString(strSec, header.depIdx, strLimit);

	loadedModels[modl->name] = modl; // Prevents circular dependency infinite loop

	// Try get dependency from cache
	if (!dependency.empty()) {
		model::model *depsModel = loadedModels[dependency];

		string file = dependency + EXT_CAAF; // Add file extension

		// Load if not already loaded
		if (depsModel == nullptr) {
			uint8_t *depsCaaf = depsFunc(file.c_str(), root);

			if (depsCaaf == nullptr)
				cerr << "Warning: could not find dependency " << dependency << " of model " << modl->name << endl;
			else {
				depsModel = loadModel(depsCaaf, root, depsFunc, device, pass);
				delete[] depsCaaf;
			}
		}

		modl->dependsOn = depsModel;
	}

	bool meshesReached = false;

	for (uint16_t i = 1; i < header.sectCnt; i++) {
		uint8_t *secStart = caaf::getSectionStart(caaf, i);
		caaf::section secType = caaf::identifySection(secStart);
		uint16_t secCnt = caaf::getSecEntryCnt(strSec);

		switch (secType) {
			case caaf::unknown:
				cerr << "Unknown section found at: " << secStart - caaf << endl;
				continue;

			case caaf::STRT:
				cerr << "Malformed CAAF: Duplicated string table." << endl;
				return modl;

			case caaf::MESH:
			case caaf::GFXP:
				if (meshesReached && modl->meshCnt != secCnt) { // MESH or GFXP found before.
					modl->meshCnt = 0;
					cerr << "Malformed CAAF: MESH and GFXP have different lengths.";
					return modl;
				}

				modl->meshCnt = secCnt;
				meshesReached = true;
				break;

			default:
				break;
		}

		if (modl->meshCnt)
			modl->meshes = new model::mesh[modl->meshCnt];
		else
			modl->meshes = nullptr;

		for (uint16_t j = 0; j < secCnt; j++) {
			uint8_t *entryPtr = caaf::getSecEntryPtr(secStart, j);

			switch (secType) {
				case caaf::MESH: {
					caaf::mesh mesh = *(caaf::mesh *)entryPtr;

					uint8_t *meshStart = entryPtr + mesh.meshPtr;
					uint32_t meshSize = mesh.vtxSize + mesh.idxSize;

					uint8_t *vbdStart = entryPtr + mesh.vbdPtr;
					uint16_t vbdCount = caaf::getSubEntryCnt(vbdStart);

					if (!vbdCount) break;

					SDL_GPUTransferBufferCreateInfo transInfo = {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
																 .size = meshSize};
					SDL_GPUTransferBuffer *transBuf = SDL_CreateGPUTransferBuffer(device, &transInfo);

					if (transBuf == nullptr) {
						cerr << SDL_GetError() << endl;
						continue;
					}

					void *mappedMem = SDL_MapGPUTransferBuffer(device, transBuf, false);

					if (mappedMem == nullptr) {
						cerr << SDL_GetError() << endl;
						SDL_ReleaseGPUTransferBuffer(device, transBuf);
						continue;
					}

					memcpy(mappedMem, meshStart, meshSize);
					SDL_UnmapGPUTransferBuffer(device, transBuf);

					SDL_GPUBufferCreateInfo vtxInfo = {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = mesh.vtxSize};
					SDL_GPUBuffer *vtxBuf = SDL_CreateGPUBuffer(device, &vtxInfo);

					if (vtxBuf == nullptr) {
						cerr << SDL_GetError() << endl;
						SDL_ReleaseGPUTransferBuffer(device, transBuf);
						continue;
					}

					SDL_GPUBufferCreateInfo idxInfo = {.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = mesh.idxSize};
					SDL_GPUBuffer *idxBuf = SDL_CreateGPUBuffer(device, &idxInfo);

					if (idxBuf == nullptr) {
						cerr << SDL_GetError() << endl;
						SDL_ReleaseGPUTransferBuffer(device, transBuf);
						continue;
					}

					SDL_GPUTransferBufferLocation src = {.transfer_buffer = transBuf, .offset = 0};
					SDL_GPUBufferRegion dst = {.buffer = vtxBuf, .offset = 0, .size = mesh.vtxSize};
					SDL_UploadToGPUBuffer(pass, &src, &dst, false);

					src.offset = mesh.vtxSize;
					dst.buffer = idxBuf;
					dst.size = mesh.idxSize;
					SDL_UploadToGPUBuffer(pass, &src, &dst, false);

					SDL_ReleaseGPUTransferBuffer(device, transBuf);
					transBuf = nullptr;

					model::mesh *mmesh = &modl->meshes[j];

					mmesh->vtxBuf = vtxBuf;
					mmesh->idxBuf = idxBuf;
					mmesh->vtxOffsCnt = vbdCount;
					mmesh->vtxOffsets = new uint32_t[vbdCount];

					memcpy(mmesh->vtxOffsets, caaf::getSubEntryPtr(vbdStart, 0), vbdCount * sizeof(uint32_t));

					break;
				}

				case caaf::GFXP: {
					caaf::gfxPip gfxpip = *(caaf::gfxPip *)entryPtr;
					break;
				}

				case caaf::TEXD: {
					caaf::texture texture = *(caaf::texture *)entryPtr;
					break;
				}

				case caaf::SAMP: {
					caaf::sampler sampler = *(caaf::sampler *)entryPtr;
					break;
				}

				default:
					break;
			}
		}
	}

	return modl;
}

// internal method
bool loadShader(uint8_t *csaf, SDL_GPUDevice *device) {}

bool loadModel(string name, SDL_GPUDevice *device, SDL_GPUCopyPass *pass)
{
	if (loadedModels.contains(name)) return true;

	string path = name + EXT_CAAF;
	uint8_t *caaf = loadCommon(path.c_str(), STORAGE_CAAF_ROOT);

	if (caaf == nullptr) return false;

	model::model *modl = loadModel(caaf, STORAGE_CAAF_ROOT, &loadCommon, device, pass);
	delete[] caaf;

	return modl != nullptr;
}

bool loadShader(string name, SDL_GPUDevice *device, SDL_GPUCopyPass *pass)
{
	if (loadedShaders.contains(name)) return true;

	string path = name + EXT_CSAF;
	uint8_t *csaf = loadCommon(path.c_str(), STORAGE_CSAF_ROOT);

	if (csaf == nullptr) return false;

	// TO-DO
}

void clearModels()
{
	for (const auto [key, value] : loadedModels)
		delete value;

	loadedModels.clear();
}

SDL_GPUShaderFormat resolvePlatformShaderFormat(SDL_GPUShaderFormat formats, SDL_GPUDevice *device)
{
	SDL_GPUShaderFormat supportedFormats = SDL_GetGPUShaderFormats(device);

	if (supportedFormats & SDL_GPU_SHADERFORMAT_MSL && formats & SDL_GPU_SHADERFORMAT_MSL)
		return SDL_GPU_SHADERFORMAT_MSL;

	if (supportedFormats & SDL_GPU_SHADERFORMAT_DXIL && formats & SDL_GPU_SHADERFORMAT_DXIL)
		return SDL_GPU_SHADERFORMAT_DXIL;

	if (supportedFormats & SDL_GPU_SHADERFORMAT_SPIRV && formats & SDL_GPU_SHADERFORMAT_SPIRV)
		return SDL_GPU_SHADERFORMAT_SPIRV;

	return SDL_GPU_SHADERFORMAT_INVALID;
}

#ifdef CAAF_ENABLE_DEBUG_TOOLS

bool readModel(string path, SDL_GPUDevice *device, SDL_GPUCopyPass *pass)
{
	filesystem::path root, file;

	{
		filesystem::path p = filesystem::path(path);
		root = p.parent_path();
		file = p.filename();
	}

	uint8_t *caaf = readCommon(file.c_str(), root.c_str());

	if (caaf == nullptr) return false;

	// Load model:
	model::model *modl = loadModel(caaf, root.c_str(), &readCommon, device, pass);
	delete[] caaf;

	return modl != nullptr;
}

#endif

} // namespace io
} // namespace engine