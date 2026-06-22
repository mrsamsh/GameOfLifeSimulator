//
// main.cpp
// GOLRenderer
//
// Created by Usama Alshughry 13.08.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#include <SDL3/SDL.h>
#include <Math.hpp>
#include <MathPrint.hpp>
#include <print>
#include <cstring>
#include <ctime>

#include "Array.hpp"

#define RAND_CHANCE 12

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#ifdef DEBUG
# define GRAPHICS_WITH_DEBUG true
#else
# define GRAPHICS_WITH_DEBUG false
#endif

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))

struct GContext
{
  static const u8* VERTEX_SHADER;
  static const u64 VERTEX_SHADER_SIZE;
  static const u8* FRAGMENT_SHADER;
  static const u64 FRAGMENT_SHADER_SIZE;
  static constexpr i32 WindowWidth = 1920 * 2, WindowHeight = 1080 * 2;
  static constexpr i32 CellSide = 1;
  static constexpr i32 gridWidth = WindowWidth / CellSide;
  static constexpr i32 gridHeight = WindowHeight / CellSide;
  static constexpr bool high_dpi = true;
  f32 current_width, current_height;
  struct {
    math::mat4 projection;
    math::mat4 view;
  } matrices;
  f32 pixel_density;
  struct Camera {
    f32 zoom = 1;
    math::vec2 target = math::vec2(WindowWidth, WindowHeight) / 2.f;
    math::vec2 offset = math::vec2(WindowWidth, WindowHeight) / 2.f;
  } camera;
  SDL_Window* window;
  SDL_GPUDevice* device;
  SDL_GPUGraphicsPipeline* pipeline;
  SDL_GPUBuffer* cell_buffer;
  SDL_GPUTransferBuffer* cell_transfer_buffer;
  using array_t = Array<i8, gridWidth * gridHeight>;
  array_t cells1;
  array_t cells2;
  array_t* current_cells = &cells1;
  array_t* next_cells = &cells2;
  array_t pixels;
  void swap_cells() {
    array_t* temp = current_cells;
    current_cells = next_cells;
    next_cells = temp;
  }
  bool space_state[2] = {}, reset_state[2] = {}, step_state[2] = {};
  u64 start_time;
  u64 frame_counter = 0;
  SDL_GPUViewport viewport;
};

void updateCamera(GContext& context);
void handleResize(GContext& context);
void toggleFullScreen(GContext& context);

void reset_cells(GContext::array_t& cells, GContext::array_t& pixels);

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
  srand(std::time(nullptr));
  static GContext context;
  *appstate = &context;

  reset_cells(*context.current_cells, context.pixels);

  SDL_Init(SDL_INIT_VIDEO);
  bool const* keyboard = SDL_GetKeyboardState(nullptr);
  context.device = SDL_CreateGPUDevice(
      SDL_GPU_SHADERFORMAT_METALLIB
      | SDL_GPU_SHADERFORMAT_DXIL
      | SDL_GPU_SHADERFORMAT_SPIRV
      , GRAPHICS_WITH_DEBUG, 0);
  context.window = SDL_CreateWindow(
      "Game of Life Simulator",
      GContext::WindowWidth, GContext::WindowHeight,
      (GContext::high_dpi ? SDL_WINDOW_HIGH_PIXEL_DENSITY : 0)
      // | SDL_WINDOW_FULLSCREEN
      | SDL_WINDOW_RESIZABLE
      );
  SDL_ClaimWindowForGPUDevice(context.device, context.window);

  context.pixel_density = SDL_GetWindowPixelDensity(context.window);

  const u8* vertex_shader_source{0};
  const u8* fragment_shader_source{0};
  u64 vertex_shader_source_size{0};
  u64 fragment_shader_source_size{0};
  auto format = SDL_GetGPUShaderFormats(context.device);

  vertex_shader_source = (const u8* const)context.VERTEX_SHADER;
  vertex_shader_source_size = context.VERTEX_SHADER_SIZE;
  fragment_shader_source = (const u8* const)context.FRAGMENT_SHADER;
  fragment_shader_source_size = context.FRAGMENT_SHADER_SIZE;

  if (format & SDL_GPU_SHADERFORMAT_METALLIB)
  {
    format = SDL_GPU_SHADERFORMAT_METALLIB;
  }
  else if (format & SDL_GPU_SHADERFORMAT_DXIL)
  {
    format = SDL_GPU_SHADERFORMAT_DXIL;
  }
  else if (format & SDL_GPU_SHADERFORMAT_SPIRV)
  {
    format = SDL_GPU_SHADERFORMAT_SPIRV;
  }
  else
  {
    puts("Unsupported");
    exit(0);
  }

  SDL_GPUShaderCreateInfo vshader_info {
    .code_size = vertex_shader_source_size,
    .code = vertex_shader_source,
    .entrypoint = "VSmain",
    .format = format,
    .stage = SDL_GPU_SHADERSTAGE_VERTEX,
    .num_uniform_buffers = 1,
  };
  SDL_GPUShader* vertex_shader = SDL_CreateGPUShader(context.device, &vshader_info);

  SDL_GPUShaderCreateInfo fshader_info {
    .code_size = fragment_shader_source_size,
    .code = fragment_shader_source,
    .entrypoint = "FSmain",
    .format = format,
    .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
    .num_storage_buffers = 1,
  };

  SDL_GPUShader* fragment_shader = SDL_CreateGPUShader(context.device, &fshader_info);

  SDL_GPUColorTargetDescription color_target_desc = {
    .format = SDL_GetGPUSwapchainTextureFormat(context.device, context.window),
  };

  SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
    .vertex_shader = vertex_shader,
    .fragment_shader = fragment_shader,
    .vertex_input_state = { },
    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
    .target_info = {
      .color_target_descriptions = &color_target_desc,
      .num_color_targets = 1
    }
  };

  context.pipeline = SDL_CreateGPUGraphicsPipeline(context.device, &pipeline_info);

  SDL_ReleaseGPUShader(context.device, vertex_shader);
  SDL_ReleaseGPUShader(context.device, fragment_shader);

  context.matrices.projection = math::mat4::ortho(0, GContext::WindowWidth, 0, GContext::WindowHeight, -10, 10);
  context.matrices.view = math::mat4::Identity();

  Array<math::vec2, GContext::gridWidth * GContext::gridHeight> transformations;
  for (int y = 0; y < GContext::gridHeight; ++y)
    for (int x = 0; x < GContext::gridWidth; ++x)
      transformations[x + y * GContext::gridWidth] = {x, y};

  SDL_GPUBufferCreateInfo buffer_create_info{
    .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
    .size = GContext::array_t::ByteCapacity(),
  };

  context.cell_buffer = SDL_CreateGPUBuffer(
      context.device,
      &buffer_create_info
      );

  SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info{
    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    .size = GContext::array_t::ByteCapacity()
  };

  context.cell_transfer_buffer = SDL_CreateGPUTransferBuffer(
      context.device, &transfer_buffer_create_info
      );

  handleResize(context);
  SDL_SyncWindow( context.window);
  SDL_RaiseWindow(context.window);
  context.start_time = SDL_GetTicksNS();
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
  GContext& context = *(GContext*)(appstate);
  switch (event->type)
  {
    case SDL_EVENT_QUIT:
      return SDL_APP_SUCCESS;
    case SDL_EVENT_KEY_DOWN:
      if (event->key.key == SDLK_ESCAPE)
        return SDL_APP_SUCCESS;
      else if (event->key.key == SDLK_RETURN && event->key.mod & SDL_KMOD_ALT)
        toggleFullScreen(context);
    break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
      handleResize(context);
    default:
    break;
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
  GContext& context = *(GContext*)appstate;
  auto calculateNext = [&context](const GContext::array_t::type_t* const current_cells, GContext::array_t::type_t* const next_cells) {
    auto const gridWidth  = GContext::WindowWidth  / context.CellSide;
    auto const gridHeight = GContext::WindowHeight / context.CellSide;
    for (auto y = 1; y < gridHeight - 1; ++y)
    {
      for (auto x = 1; x < gridWidth - 1; ++x)
      {
        auto cellIndex = x + y * gridWidth;
                                // east west
        next_cells[cellIndex] = current_cells[cellIndex + 1]
                              + current_cells[cellIndex - 1]
                                // north and south
                              + current_cells[cellIndex + gridWidth]
                              + current_cells[cellIndex - gridWidth]
                                // ne and se
                              + current_cells[cellIndex + gridWidth + 1]
                              + current_cells[cellIndex - gridWidth + 1]
                                // nw and sw
                              + current_cells[cellIndex + gridWidth - 1]
                              + current_cells[cellIndex - gridWidth - 1];
      }
    }

    for (auto x = 0; x < gridWidth; ++x)
    {
      auto nIndex = x, sIndex = x + (gridHeight - 1) * gridWidth, last_row = gridWidth * (gridHeight - 1);
      auto x_pls_1_mod = (x + 1) % gridWidth;
      auto x_min_1_mod = (x - 1 + gridWidth) % gridWidth;

      next_cells[nIndex] = current_cells[x_pls_1_mod]
                         + current_cells[x_min_1_mod]

                         + current_cells[x + gridWidth]
                         + current_cells[sIndex]

                         + current_cells[x_pls_1_mod + gridWidth]
                         + current_cells[x_pls_1_mod + last_row]

                         + current_cells[x_min_1_mod + gridWidth]
                         + current_cells[x_min_1_mod + last_row];

      next_cells[sIndex] = current_cells[x_pls_1_mod + last_row]
                         + current_cells[x_min_1_mod + last_row]

                         + current_cells[x]
                         + current_cells[sIndex - gridWidth]

                         + current_cells[x_pls_1_mod]
                         + current_cells[x_min_1_mod]

                         + current_cells[x_pls_1_mod + gridWidth * (gridHeight - 2)]
                         + current_cells[x_min_1_mod + gridWidth * (gridHeight - 2)];
    }

    for (auto y = 1; y < (gridHeight - 1); ++y)
    {
      auto eIndex = y * gridWidth, wIndex = eIndex + gridWidth - 1;
      // north and south
      next_cells[eIndex] = current_cells[eIndex - gridWidth]
                         + current_cells[eIndex + gridWidth]
      // east and west
                         + current_cells[eIndex + 1]
                         + current_cells[wIndex]
      // ne and se
                         + current_cells[eIndex - gridWidth + 1]
                         + current_cells[eIndex + gridWidth + 1]
      // nw and sw
                         + current_cells[wIndex - gridWidth]
                         + current_cells[wIndex + gridWidth];

      // north and south
      next_cells[wIndex] = current_cells[wIndex - gridWidth]
                         + current_cells[wIndex + gridWidth]

      // east and west
                         + current_cells[eIndex]
                         + current_cells[wIndex - 1]
      // nw and sw
                         + current_cells[wIndex - gridWidth - 1]
                         + current_cells[wIndex + gridWidth - 1]
      // ne and se
                         + current_cells[eIndex - gridWidth]
                         + current_cells[eIndex + gridWidth];
    }


    GContext::array_t::type_t* pixels = context.pixels.data();
    for (usz i = 0; i < gridWidth * gridHeight; ++i) {
      auto cc = current_cells[i];
      GContext::array_t::type_t sc = next_cells[i];
      next_cells[i] = 0;
      GContext::array_t::type_t& result = pixels[i];
      if (cc == 1) {
        switch (sc) {
        case 2:
        case 3:
          next_cells[i] = 1;
          result = 21;
          break;
        default:
          result = 0;
          break;
        }
      } else {
        if (sc == 3) {
          next_cells[i] = 1;
          result = 21;
        } else {
          result = std::min(20, result + 1);
        }
      }
    }
  };

  static bool const* keyboard = SDL_GetKeyboardState(nullptr);
  static bool updating = true;
  bool* space = context.space_state;
  space[0] = space[1];
  space[1] = keyboard[SDL_SCANCODE_SPACE];
  if (space[1] && !space[0]) {
    updating = !updating;
  }

  bool* reset = context.reset_state;
  reset[0] = reset[1];
  reset[1] = keyboard[SDL_SCANCODE_R];

  if (reset[1] && !reset[0]) {
    reset_cells(*context.current_cells, context.pixels);
  }

  bool* step = context.step_state;
  step[0] = step[1];
  step[1] = keyboard[SDL_SCANCODE_E];

  if ((step[1] && ! step[0]) || keyboard[SDL_SCANCODE_Q]) {
    updating = false;
    calculateNext(context.current_cells->data(), context.next_cells->data());
    context.swap_cells();
  }

  math::vec3 mousepos {};
  auto state = SDL_GetMouseState(&mousepos.x, &mousepos.y);
  static bool last_time, this_time;
  last_time = this_time;
  this_time = (state & SDL_BUTTON_LMASK) != 0;
  // mousepos = mouseToNormal.transform(mousepos);
  if (this_time && !last_time) {
    mousepos = context.matrices.view.inverse().transform(mousepos);
    u32 xx = floor(mousepos.x) / GContext::CellSide;
    u32 yy = floor(mousepos.y) / GContext::CellSide;
    u32 i = xx + yy * (GContext::WindowWidth / GContext::CellSide);
    auto& clicked_cell = (*context.current_cells)[i];
    auto& pixel = context.pixels[i];
    clicked_cell = clicked_cell == 1 ? 0 : 1;
    pixel = pixel == 21 ? 20 : 21;
  }

  f32 zoomF = 0;
  math::vec2 camVel;
  static constexpr f64 FPS = 60.0;
  static constexpr f32 CamSpeed = 300;
  static constexpr math::Time Delta = math::seconds(1.0 / FPS);
  if (keyboard[SDL_SCANCODE_K]) zoomF += 2;
  if (keyboard[SDL_SCANCODE_J]) zoomF -= 2;
  if (keyboard[SDL_SCANCODE_D]) camVel.x += 1;
  if (keyboard[SDL_SCANCODE_A]) camVel.x -= 1;
  if (keyboard[SDL_SCANCODE_S]) camVel.y += 1;
  if (keyboard[SDL_SCANCODE_W]) camVel.y -= 1;
  if (zoomF != 0 || !math::isZero(camVel)) {
    context.camera.zoom *= (1 + zoomF * context.pixel_density * Delta.asMilliseconds() / 4000.f);
    context.camera.target += camVel * context.pixel_density * CamSpeed * Delta.asSeconds()
                                     * (1.f / (context.camera.zoom));
    updateCamera(context);
  }

  // update here

  // draw here
  // first upload pass
  
  void* map = SDL_MapGPUTransferBuffer(context.device, context.cell_transfer_buffer, false);
  memcpy(map, context.pixels.data(), context.pixels.ByteCapacity());
  SDL_UnmapGPUTransferBuffer(context.device, context.cell_transfer_buffer);

  SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(context.device);
  SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);

  static SDL_GPUTransferBufferLocation location{
    .transfer_buffer = context.cell_transfer_buffer,
    .offset = 0
  };

  static SDL_GPUBufferRegion region{
    .buffer = context.cell_buffer,
    .offset = 0,
    .size = GContext::array_t::ByteCapacity()
  };

  SDL_UploadToGPUBuffer(
      copy_pass,
      &location,
      &region,
      false
      );

  SDL_EndGPUCopyPass(copy_pass);

  SDL_GPUTexture* swapchain_texture;
  u32 width, height;
  SDL_WaitAndAcquireGPUSwapchainTexture(
      command_buffer,
      context.window,
      &swapchain_texture,
      &width,
      &height
      );

  SDL_GPUColorTargetInfo color_target_info{
    .texture = swapchain_texture,
    .clear_color = {0.f, 0.025f, 0.2f, 1.f},
    .load_op = SDL_GPU_LOADOP_CLEAR,
    .store_op = SDL_GPU_STOREOP_STORE,
  };

  SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(
      command_buffer,
      &color_target_info,
      1,
      nullptr
      );

  SDL_BindGPUGraphicsPipeline(render_pass, context.pipeline);

  SDL_GPUBufferBinding buffer_bind[]{
    {.buffer = context.cell_buffer},
  };

  SDL_BindGPUFragmentStorageBuffers(render_pass, 0, &context.cell_buffer, 1);

  SDL_SetGPUViewport(render_pass, &context.viewport);

  struct {
    math::mat4 projection;
    math::vec2 windowSize;
    math::vec2 gridSize;
    f32        cellSide;
  } ubo = {context.matrices.projection * context.matrices.view, {GContext::WindowWidth, GContext::WindowHeight}, {GContext::gridWidth , GContext::gridHeight}, GContext::CellSide};

  SDL_PushGPUVertexUniformData(command_buffer, 0, &ubo, sizeof(ubo));
  SDL_DrawGPUPrimitives(render_pass, 6, 1, 0, 0);
  SDL_EndGPURenderPass(render_pass);
  SDL_SubmitGPUCommandBuffer(command_buffer);


  u64 start = SDL_GetTicksNS();
  if (updating)
  {
    calculateNext(context.current_cells->data(), context.next_cells->data());
  }
  std::println("update elapsed: {} ms", (SDL_GetTicksNS() - start) * 1.e-6);


  if (updating)
    context.swap_cells();
  // if (elapsed < Delta)
  //   SDL_DelayPrecise((Delta - elapsed).asNanoseconds());

  context.frame_counter++;
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
  GContext& context = *(GContext*)appstate;
  u64 elapsed = SDL_GetTicksNS() - context.start_time;
  u64 ns_per_frame = elapsed / context.frame_counter;
  std::println("total elapsed   : {:7.3f} sec", elapsed * 1.e-9);
  std::println("total frames    : {:3d} frame", context.frame_counter);
  std::println("avg ms per frame: {:7.3f} ms", ns_per_frame * 1.e-6);
  std::println("fps             : {:7.3f}", context.frame_counter / (elapsed * 1.e-9));
  SDL_ReleaseGPUBuffer(context.device, context.cell_buffer);
  SDL_ReleaseGPUTransferBuffer(context.device, context.cell_transfer_buffer);
  SDL_DestroyGPUDevice(context.device);
  SDL_DestroyWindow(context.window);
  SDL_Quit();
}

void updateCamera(GContext& context)
{
  GContext::Camera& camera = context.camera;
  math::vec2 min = {0, 0};
  math::vec2 max = {GContext::WindowWidth, GContext::WindowHeight};
  math::vec2 current_size = {context.current_width, context.current_height};
  math::vec2 temp = current_size / max;
  f32 min_zoom = math::max(temp.x, temp.y);
  camera.zoom = math::clamp(camera.zoom, min_zoom, 30.f);

  math::vec2 apparent_offset = camera.offset / camera.zoom;

  // limit the boundaries
  camera.target.x = math::clamp(
      camera.target.x,
      min.x + apparent_offset.x,
      max.x - apparent_offset.x
  );

  camera.target.y = math::clamp(
      camera.target.y,
      min.y + apparent_offset.y,
      max.y - apparent_offset.y
  );

  context.matrices.view = math::mat4::Identity()
    .translate({camera.offset.x, camera.offset.y, 0})
    .scale({camera.zoom, camera.zoom, 1})
    .translate({-camera.target.x, -camera.target.y, 0});
}

void reset_cells(GContext::array_t& cells, GContext::array_t& pixels) {

  for (usz i = 0, count = cells.size(); i < count; ++i) {
    if (rand() % RAND_CHANCE == 3) {
      cells[i] = 1;
      pixels[i] = 21;
    } else {
      cells[i] = 0;
      pixels[i] = 20;
    }
  }
}

void handleResize(GContext& context)
{
  i32 width, height;
  SDL_GetWindowSize(context.window, &width, &height);
  context.matrices.projection = math::mat4::ortho(
      0, width,
      0, height,
      -10, 10
      );
  context.current_width = width;
  context.current_height = height;
  context.camera.offset = math::vec2(width, height) / 2.f;
  updateCamera(context);
  SDL_GetWindowSizeInPixels(context.window, &width, &height);
  context.viewport = {0, 0, (f32)width, (f32)height};
}

void toggleFullScreen(GContext& context)
{
  u32 flags = SDL_GetWindowFlags(context.window);

  SDL_SetWindowFullscreen(context.window, !(flags & SDL_WINDOW_FULLSCREEN));
}

#include "../generated/Shader.vert.hpp"
const u8* GContext::VERTEX_SHADER = Shader_vert;
const u64 GContext::VERTEX_SHADER_SIZE = Shader_vert_len;

#include "../generated/Shader.frag.hpp"
const u8* GContext::FRAGMENT_SHADER = Shader_frag;
const u64 GContext::FRAGMENT_SHADER_SIZE = Shader_frag_len;

