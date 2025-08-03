#define SDL_MAIN_USE_CALLBACKS

#include "engine/io.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <format>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <string>

using namespace std;

static SDL_Window *window = nullptr;
static SDL_GPUDevice *device = nullptr;

static Uint64 lastFrame, currentFrame;
static double frequency, delta;

static bool isDemoWindowOpened = false;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Create SDL window graphics context:
	float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
	window = SDL_CreateWindow("CAAF Editor", (int)(1280 * scale), (int)(720 * scale), flags);

	if (window == nullptr) {
		SDL_Log("Couldn't create window: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Create GPU Device:
	device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB,
								 true, nullptr);

	if (device == nullptr) {
		SDL_Log("Couldn't create GPU device: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Claim window for GPU Device:
	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		SDL_Log("Failed to claim window for GPU device: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

	lastFrame = 0;
	currentFrame = SDL_GetPerformanceCounter();
	frequency = (double)SDL_GetPerformanceFrequency();

	// Init ImGui:
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui::StyleColorsDark();
	ImGuiStyle &style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
	style.FontScaleDpi = scale;

	// Setup backend:
	ImGui_ImplSDL3_InitForSDLGPU(window);
	ImGui_ImplSDLGPU3_InitInfo initinfo = {device, SDL_GetGPUSwapchainTextureFormat(device, window),
										   SDL_GPU_SAMPLECOUNT_1};
	ImGui_ImplSDLGPU3_Init(&initinfo);

	engine::io::loadActor("");

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	ImGui_ImplSDL3_ProcessEvent(event);

	if (event->type == SDL_EVENT_QUIT) {
		return SDL_APP_SUCCESS; // end the program, reporting success to the OS.
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult render()
{
	ImGui::Render();
	ImDrawData *drawData = ImGui::GetDrawData();
	SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(device);
	SDL_GPUTexture *swapchainTexture;
	SDL_AcquireGPUSwapchainTexture(cmdbuf, window, &swapchainTexture, nullptr, nullptr);

	if (swapchainTexture == nullptr) {
		SDL_SubmitGPUCommandBuffer(cmdbuf);
		return SDL_APP_CONTINUE;
	}

	ImGui_ImplSDLGPU3_PrepareDrawData(drawData, cmdbuf);

	SDL_GPUColorTargetInfo colorTargetInfo = {.texture = swapchainTexture,
											  .clear_color = {0.059f, 0.059f, 0.059f, 1.0f},
											  .load_op = SDL_GPU_LOADOP_CLEAR,
											  .store_op = SDL_GPU_STOREOP_STORE};

	SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, nullptr);

	ImGui_ImplSDLGPU3_RenderDrawData(drawData, cmdbuf, renderPass);

	SDL_EndGPURenderPass(renderPass);
	SDL_SubmitGPUCommandBuffer(cmdbuf);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	lastFrame = currentFrame;
	currentFrame = SDL_GetPerformanceCounter();
	delta = (currentFrame - lastFrame) / frequency;

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::BeginMenu("New")) {
				if (ImGui::MenuItem("Actor", "Ctrl+N")) {
					//
				}

				if (ImGui::MenuItem("Dependency")) {
					//
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Open", "Ctrl+O")) {
				//
			}

			if (ImGui::MenuItem("Save", "Ctrl+S")) {
				//
			}

			if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
				//
			}

			if (ImGui::MenuItem("Close")) {
				//
			}

			ImGui::Separator();

			if (ImGui::MenuItem("Quit")) {
				return SDL_APP_SUCCESS;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Demo")) {
			isDemoWindowOpened ^= true;
			ImGui::EndMenu();
		}

		string deltaStr = format("Delta: {} s", delta);
		const char *deltaCStr = deltaStr.c_str();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(deltaCStr).x -
							 ImGui::GetStyle().FramePadding.x);
		ImGui::Text("%s", deltaCStr);

		ImGui::EndMainMenuBar();
	}

	if (isDemoWindowOpened) ImGui::ShowDemoWindow(&isDemoWindowOpened);

	return render();
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	SDL_WaitForGPUIdle(device);
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyGPUDevice(device);
	SDL_DestroyWindow(window);
}
