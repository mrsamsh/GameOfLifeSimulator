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
#include <thread>

#include "Array.hpp"
#include "Clock.hpp"

#define RAND_CHANCE 12

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

struct GContext
{
  static std::string_view VERTEX_SHADER;
  static std::string_view FRAGMENT_SHADER;
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
  using array_t = Array<i32, gridWidth * gridHeight>;
  array_t cells1;
  array_t cells2;
  array_t* current_cells = &cells1;
  array_t* next_cells = &cells2;
  void swap_cells() {
    array_t* temp = current_cells;
    current_cells = next_cells;
    next_cells = temp;
  }
  bool space_state[2] = {}, reset_state[2] = {}, step_state[2] = {};
  SDL_GPUViewport viewport;
};

void updateCamera(GContext* context);
void handleResize(GContext* context);
void toggleFullScreen(GContext* context);

void reset_cells(GContext::array_t& cells, unsigned int seed = std::time(nullptr));

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
  srand(std::time(nullptr));
  GContext* context = new GContext;
  *appstate = context;

  // GContext* context = (GContext*)(*appstate);
  reset_cells(*context->current_cells);

  SDL_Init(SDL_INIT_VIDEO);
  bool const* keyboard = SDL_GetKeyboardState(nullptr);
  context->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, false, 0);
  context->window = SDL_CreateWindow(
      "Test",
      GContext::WindowWidth, GContext::WindowHeight,
      (GContext::high_dpi ? SDL_WINDOW_HIGH_PIXEL_DENSITY : 0)
      // | SDL_WINDOW_FULLSCREEN
      | SDL_WINDOW_RESIZABLE
      );
  SDL_ClaimWindowForGPUDevice(context->device, context->window);

  context->pixel_density = SDL_GetWindowPixelDensity(context->window);

  SDL_GPUShaderCreateInfo vshader_info {
    .code_size = GContext::VERTEX_SHADER.size(),
    .code = (u8 const*)GContext::VERTEX_SHADER.data(),
    .entrypoint = "VSmain",
    .format = SDL_GPU_SHADERFORMAT_MSL,
    .stage = SDL_GPU_SHADERSTAGE_VERTEX,
    .num_uniform_buffers = 1,
  };
  SDL_GPUShader* vertex_shader = SDL_CreateGPUShader(context->device, &vshader_info);

  SDL_GPUShaderCreateInfo fshader_info {
    .code_size = GContext::FRAGMENT_SHADER.size(),
    .code = (u8 const*)GContext::FRAGMENT_SHADER.data(),
    .entrypoint = "FSmain",
    .format = SDL_GPU_SHADERFORMAT_MSL,
    .stage = SDL_GPU_SHADERSTAGE_FRAGMENT,
  };

  SDL_GPUShader* fragment_shader = SDL_CreateGPUShader(context->device, &fshader_info);

  SDL_GPUVertexBufferDescription vb_desc = {
    .slot = 0,
    .pitch = sizeof(i32),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_INSTANCE,
  };

  SDL_GPUVertexAttribute v_attrib = {
    .location = 0,
    .buffer_slot = 0,
    .format = SDL_GPU_VERTEXELEMENTFORMAT_INT,
    .offset = 0
  };

  SDL_GPUVertexInputState vertex_input_state = {
    .vertex_buffer_descriptions = &vb_desc,
    .num_vertex_buffers = 1,
    .vertex_attributes = &v_attrib,
    .num_vertex_attributes = 1
  };

  SDL_GPUColorTargetDescription color_target_desc = {
    .format = SDL_GetGPUSwapchainTextureFormat(context->device, context->window),
  };

  SDL_GPUGraphicsPipelineTargetInfo pipeline_target_info = {
    .color_target_descriptions = &color_target_desc,
    .num_color_targets = 1,
  };

  SDL_GPUGraphicsPipelineCreateInfo pipeline_info = {
    .vertex_shader = vertex_shader,
    .fragment_shader = fragment_shader,
    .vertex_input_state = vertex_input_state,
    .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
    .target_info = pipeline_target_info
  };


  context->pipeline = SDL_CreateGPUGraphicsPipeline(context->device, &pipeline_info);
  SDL_ReleaseGPUShader(context->device, vertex_shader);
  SDL_ReleaseGPUShader(context->device, fragment_shader);

  context->matrices.projection = math::mat4::ortho(0, GContext::WindowWidth, 0, GContext::WindowHeight, -10, 10);
  context->matrices.view = math::mat4::Identity();

  SDL_GPUBufferCreateInfo buffer_create_info{
    .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
    .size = GContext::array_t::ByteCapacity(),
  };

  context->cell_buffer = SDL_CreateGPUBuffer(
      context->device,
      &buffer_create_info
      );

  SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info{
    .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
    .size = GContext::array_t::ByteCapacity()
  };

  context->cell_transfer_buffer = SDL_CreateGPUTransferBuffer(
      context->device, &transfer_buffer_create_info
      );

  handleResize(context);
  SDL_SyncWindow( context->window);
  SDL_RaiseWindow(context->window);
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
  GContext* context = (GContext*)appstate;
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
  GContext* context = (GContext*)appstate;
  static math::Time begin = 0_sec;

  auto calculateNext = [&](GContext::array_t const& current_cells, GContext::array_t& next_cells) {
    const int gridWidth  = GContext::WindowWidth  / context->CellSide;
    const int gridHeight = GContext::WindowHeight / context->CellSide;
    std::fill(next_cells.begin(), next_cells.end(), 0);
    {
      std::vector<std::jthread> thread_pool;
      static constexpr u64 ThreadCount = 20,
                           Chunk = GContext::gridHeight / ThreadCount;
      for (int j = 0; j < ThreadCount; ++j)
      {
        thread_pool.emplace_back([j,&current_cells,&next_cells]{
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            if (y == GContext::gridHeight - 1) continue;
            for (auto x = 0; x < GContext::gridWidth; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex + GContext::gridWidth] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            if (y == 0) continue;
            for (auto x = 0; x < GContext::gridWidth; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex - GContext::gridWidth] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            for (auto x = 0; x < GContext::gridWidth - 1; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex + 1] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            for (auto x = 1; x < GContext::gridWidth; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex - 1] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            if (y == 0) continue;
            for (auto x = 1; x < GContext::gridWidth; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex - 1 - GContext::gridWidth] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            if (y == GContext::gridHeight - 1) continue;
            for (auto x = 1; x < GContext::gridWidth; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex - 1 + GContext::gridWidth] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            if (y == GContext::gridHeight - 1) continue;
            for (auto x = 0; x < GContext::gridWidth - 1; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex + 1 + GContext::gridWidth] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
          for (auto y = Chunk * j; y < Chunk * (j + 1); ++y) {
            if (y == 0) continue;
            for (auto x = 0; x < GContext::gridWidth - 1; ++x) {
              auto cellIndex = x + y * GContext::gridWidth;
              auto value = current_cells[cellIndex + 1 - GContext::gridWidth] == 1 ? 1 : 0;
              next_cells[cellIndex] += value;
            }
          }
        });
      }
    }

    // edge cases?
    for (auto x = 0; x < gridWidth; x++) {
      auto targetIndex = gridWidth * (gridHeight - 1) + x;
      auto value = current_cells[targetIndex] == 1 ? 1 : 0;
      next_cells[x] += value;
    }
    for (auto x = 0; x < gridWidth; x++) {
      auto value = current_cells[x] == 1 ? 1 : 0;
      auto targetIndex = gridWidth * (gridHeight - 1) + x;
      next_cells[targetIndex] += value;
    }
    for (auto x = 0; x < gridWidth; x++) {
      auto targetIndex = gridWidth * (gridHeight - 1) + ((x + 1) % gridWidth);
      auto value = current_cells[targetIndex] == 1 ? 1 : 0;
      next_cells[x] += value;
    }
    for (auto x = 0; x < gridWidth; x++) {
      auto value = current_cells[x] == 1 ? 1 : 0;
      auto targetIndex = gridWidth * (gridHeight - 1) + ((x + 1) % gridWidth);
      next_cells[targetIndex] += value;
    }
    for (auto x = 0; x < gridWidth; x++) {
      auto targetIndex = gridWidth * (gridHeight - 1) + ((gridWidth + x - 1) % gridWidth);
      auto value = current_cells[targetIndex] == 1 ? 1 : 0;
      next_cells[x] += value;
    }
    for (auto x = 0; x < gridWidth; x++) {
      auto value = current_cells[x] == 1 ? 1 : 0;
      auto targetIndex = gridWidth * (gridHeight - 1) + ((gridWidth + x - 1) % gridWidth);
      next_cells[targetIndex] += value;
    }

    // for (auto y = 0; y < gridHeight; ++y) {
    //   auto targetIndex = gridWidth * y;
    //   auto value = current_cells[targetIndex] == 1 ? 1 : 0;
    //   next_cells[targetIndex + gridWidth -1] += value;
    // }
    // for (auto y = 0; y < gridHeight; ++y) {
    //   auto targetIndex = gridWidth * y + gridWidth - 1;
    //   auto value = current_cells[targetIndex] == 1 ? 1 : 0;
    //   next_cells[gridWidth * y] += value;
    // }
    // for (auto y = 0; y < gridHeight; ++y) {
    //   auto targetIndex = gridWidth * ((y + 1) % gridHeight);
    //   auto value = current_cells[targetIndex] == 1 ? 1 : 0;
    //   next_cells[y * gridWidth] += value;
    // }
    // for (auto y = 0; y < gridHeight; ++y) {
    //   auto targetIndex = gridWidth * y + gridWidth - 1;
    //   auto value = current_cells[targetIndex] == 1 ? 1 : 0;
    //   next_cells[gridWidth * y] += value;
    // }
    // for (auto y = 0; y < gridHeight; ++y) {
    //   auto targetIndex = gridWidth * ((gridHeight + y - 1) % gridHeight);
    //   auto value = current_cells[targetIndex] == 1 ? 1 : 0;
    //   next_cells[targetIndex + gridWidth -1] += value;
    // }

    {
      static constexpr int ThreadCount = 8;
      std::vector<std::jthread> thread_pool;
      thread_pool.reserve(ThreadCount);
      int chunk = context->next_cells->size() / ThreadCount;

      for (int j = 0; j < ThreadCount; ++j)
      {
        thread_pool.emplace_back([chunk,j](GContext::array_t const& current_cells, GContext::array_t& next_cells){
          for (usz i = j * chunk; i < (j + 1) * chunk; ++i) {
            auto cc = current_cells[i];
            i8 sc = next_cells[i];
            if (cc == 1) {
              switch (sc) {
              case 2:
              case 3:
                next_cells[i] = 1;
                break;
              default:
                next_cells[i] = -20;
                break;
              }
            } else {
              if (sc == 3) {
                next_cells[i] = 1;
              } else {
                next_cells[i] = std::min(0, cc + 1);
              }
            }
          }
        }, std::ref(current_cells), std::ref(next_cells));
      }
    }
  };

  static bool const* keyboard = SDL_GetKeyboardState(nullptr);
  static bool updating = true;
  bool* space = context->space_state;
  space[0] = space[1];
  space[1] = keyboard[SDL_SCANCODE_SPACE];
  if (space[1] && !space[0]) {
    updating = !updating;
  }

  bool* reset = context->reset_state;
  reset[0] = reset[1];
  reset[1] = keyboard[SDL_SCANCODE_R];

  if (reset[1] && !reset[0]) {
    reset_cells(*context->current_cells, 5);
  }

  bool* step = context->step_state;
  step[0] = step[1];
  step[1] = keyboard[SDL_SCANCODE_E];

  if ((step[1] && ! step[0]) || keyboard[SDL_SCANCODE_Q]) {
    updating = false;
    calculateNext(*context->current_cells, *context->next_cells);
    context->swap_cells();
  }

  math::vec3 mousepos {};
  auto state = SDL_GetMouseState(&mousepos.x, &mousepos.y);
  static bool last_time, this_time;
  last_time = this_time;
  this_time = (state & SDL_BUTTON_LMASK) != 0;
  // mousepos = mouseToNormal.transform(mousepos);
  if (this_time && !last_time) {
    mousepos = context->matrices.view.inverse().transform(mousepos);
    u32 xx = floor(mousepos.x) / GContext::CellSide;
    u32 yy = floor(mousepos.y) / GContext::CellSide;
    u32 i = xx + yy * (GContext::WindowWidth / GContext::CellSide);
    auto& clicked_cell = (*context->current_cells)[i];
    clicked_cell = clicked_cell == 1 ? 0 : 1;
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
    context->camera.zoom *= (1 + zoomF * context->pixel_density * Delta.asMilliseconds() / 4000.f);
    context->camera.target += camVel * context->pixel_density * CamSpeed * Delta.asSeconds()
                                     * (1.f / (context->camera.zoom));
    updateCamera(context);
  }

  // update here
  if (updating)
  {
    calculateNext(*context->current_cells, *context->next_cells);
  }
  // -----------------------------------

  // draw here
  // first upload pass
  SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(context->device);
  SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(command_buffer);
  i32* map = (i32*)SDL_MapGPUTransferBuffer(context->device, context->cell_transfer_buffer, false);
  memcpy(map, context->current_cells->data(), context->current_cells->ByteCapacity());
  SDL_UnmapGPUTransferBuffer(context->device, context->cell_transfer_buffer);

  SDL_GPUTransferBufferLocation location{
    .transfer_buffer = context->cell_transfer_buffer,
    .offset = 0
  };
  SDL_GPUBufferRegion region{
    .buffer = context->cell_buffer,
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
      context->window,
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

  SDL_BindGPUGraphicsPipeline(render_pass, context->pipeline);
  SDL_GPUBufferBinding buffer_bind{
    .buffer = context->cell_buffer,
  };
  SDL_BindGPUVertexBuffers(render_pass, 0, &buffer_bind, 1);

  SDL_SetGPUViewport(render_pass, &context->viewport);

  struct {
    math::mat4 projection;
    math::vec2 gridSize;
    f32        cellSide;
  } ubo = {context->matrices.projection * context->matrices.view, {GContext::gridWidth , GContext::gridHeight}, GContext::CellSide};

  SDL_PushGPUVertexUniformData(command_buffer, 0, &ubo, sizeof(ubo));
  SDL_DrawGPUPrimitives(render_pass, 6, context->current_cells->size(), 0, 0);
  SDL_EndGPURenderPass(render_pass);
  SDL_SubmitGPUCommandBuffer(command_buffer);

  if (updating)
    context->swap_cells();
  auto elapsed = math::Clock::Now() - begin;
  if (elapsed < Delta)
    SDL_DelayPrecise((Delta - elapsed).asNanoseconds());
  begin = math::Clock::Now();

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
  GContext* context = (GContext*)appstate;
  SDL_ReleaseGPUBuffer(context->device, context->cell_buffer);
  SDL_ReleaseGPUTransferBuffer(context->device, context->cell_transfer_buffer);
  SDL_DestroyGPUDevice(context->device);
  SDL_DestroyWindow(context->window);
  SDL_Quit();
  delete context;
}

void updateCamera(GContext* context)
{
  GContext::Camera& camera = context->camera;
  math::vec2 min = {0, 0};
  math::vec2 max = {GContext::WindowWidth, GContext::WindowHeight};
  math::vec2 current_size = {context->current_width, context->current_height};
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

  context->matrices.view = math::mat4::Identity()
    .translate({camera.offset.x, camera.offset.y, 0})
    .scale({camera.zoom, camera.zoom, 1})
    .translate({-camera.target.x, -camera.target.y, 0});
}

void reset_cells(GContext::array_t& cells, unsigned int seed) {

  // srand(seed);
  for (auto& cell : cells) {
    if (rand() % RAND_CHANCE == 3) {
      cell = 1;
    } else {
      cell = 0;
    }
  }
}

void handleResize(GContext* context)
{
  i32 width, height;
  SDL_GetWindowSize(context->window, &width, &height);
  context->matrices.projection = math::mat4::ortho(
      0, width,
      0, height,
      -10, 10
      );
  context->current_width = width;
  context->current_height = height;
  context->camera.offset = math::vec2(width, height) / 2.f;
  updateCamera(context);
  SDL_GetWindowSizeInPixels(context->window, &width, &height);
  context->viewport = {0, 0, (f32)width, (f32)height};
}

void toggleFullScreen(GContext* context)
{
  u32 flags = SDL_GetWindowFlags(context->window);

  SDL_SetWindowFullscreen(context->window, !(flags & SDL_WINDOW_FULLSCREEN));
}

std::string_view GContext::VERTEX_SHADER = R"(
#include <metal_stdlib>
using namespace metal;

struct UniformBufferObject
{
  float4x4 projection;
  float2   gridSize;
  float    cellSide;
};

struct VertexIn
{
  int color [[attribute(0)]];
};

struct VertexOut
{
  float4 position [[position]];
  float4 color;
};


vertex VertexOut VSmain(VertexIn in [[stage_in]],
                        uint vertexID [[vertex_id]],
                        uint instanceID [[instance_id]],
                        constant UniformBufferObject& ubo [[buffer(0)]])
{
  const float2 VertexPositions[6] = {
    {0.0, 0.0},
    {0.8, 0.0},
    {0.8, 0.8},
    {0.0, 0.8},
    {0.0, 0.0},
    {0.8, 0.8}
  };
  VertexOut out;
  switch (in.color)
  {
    case 1:
      out.color = float4(1, 1, 1, 1);
      break;
    case 0:
      out.color = float4(0, 0.0125, 0.1, 1);
      break;
    default:
      out.color = float4(0, 0.1, 0.8, 1) * in.color / -20.0;
      break;
  }

  float2 translation;
  translation.x = (instanceID % (int)ubo.gridSize.x) * ubo.cellSide;
  translation.y = (int(instanceID) / int(ubo.gridSize.x)) * ubo.cellSide;
  float2 position = (VertexPositions[vertexID] * ubo.cellSide + translation);
  out.position = ubo.projection * float4(position, 0, 1);
  return out;
}
)";

std::string_view GContext::FRAGMENT_SHADER = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut
{
  float4 position [[position]];
  float4 color;
};


fragment float4 FSmain(VertexOut in [[stage_in]])
{
  return in.color;
}
)";
