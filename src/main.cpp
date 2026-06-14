//
// main.cpp
// GOLRenderer
//
// Created by Usama Alshughry 13.08.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <Math.hpp>
#include <MathPrint.hpp>
#include <thread>

#include "Array.hpp"
#include "ShaderProgram.hpp"
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
  math::mat4 projection;
  math::mat4 view;
  // math::mat4 currentProjection;
  f32 pixel_density;
  struct Camera {
    f32 zoom = 1;
    math::vec2 target = math::vec2(WindowWidth, WindowHeight) / 2.f;
    math::vec2 offset = math::vec2(WindowWidth, WindowHeight) / 2.f;
  } camera;
  SDL_Window* window;
  ShaderProgram program;
  u32 VAO, VBO, VBO_Transform;
  using array_t = Array<i8, gridWidth * gridHeight>;
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
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  context->window = SDL_CreateWindow(
      "Test",
      GContext::WindowWidth, GContext::WindowHeight,
      SDL_WINDOW_OPENGL
      | (GContext::high_dpi ? SDL_WINDOW_HIGH_PIXEL_DENSITY : 0)
      // | SDL_WINDOW_FULLSCREEN
      | SDL_WINDOW_RESIZABLE
      );
  SDL_GL_CreateContext(context->window);
  gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
  printf("%s\n", glGetString(GL_VERSION));

  context->pixel_density = SDL_GetWindowPixelDensity(context->window);

  context->program.loadProgramFromString({
      {GL_VERTEX_SHADER,   GContext::VERTEX_SHADER},
      {GL_FRAGMENT_SHADER, GContext::FRAGMENT_SHADER}
  });

  context->projection = math::mat4::ortho(0, GContext::WindowWidth, 0, GContext::WindowHeight, -10, 10);
  context->view = math::mat4::Identity();
  // context->currentProjection = context->projection;
  context->program.use().set("projection", context->projection * context->view);
  context->program.use().set("gridSize", math::vec2(GContext::gridWidth, GContext::gridHeight));
  context->program.use().set("cellSide", (f32)context->CellSide);

  glGenVertexArrays(1, &context->VAO);
  glBindVertexArray(context->VAO);

  glGenBuffers(1, &context->VBO_Transform);
  Array<math::vec2, GContext::gridWidth * GContext::gridHeight> transformations;
  for (int y = 0; y < GContext::gridHeight; ++y)
    for (int x = 0; x < GContext::gridWidth; ++x)
      transformations[x + y * GContext::gridWidth] = {x, y};
  glBindBuffer(GL_ARRAY_BUFFER, context->VBO_Transform);
  glBufferData(GL_ARRAY_BUFFER, transformations.ByteCapacity(), transformations.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(math::vec2), 0);
  glVertexAttribDivisor(1, 1);

  glGenBuffers(1, &context->VBO);
  glBindBuffer(GL_ARRAY_BUFFER, context->VBO);
  glBufferData(GL_ARRAY_BUFFER, context->current_cells->ByteCapacity(), nullptr, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_BYTE, false, sizeof(i8), 0);
  glVertexAttribDivisor(0, 1);

  glClearColor(0.0, 0.025, 0.2, 1);
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
    mousepos = context->view.inverse().transform(mousepos);
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
  u64 start = SDL_GetTicksNS();
  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(context->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, context->VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, context->current_cells->ByteCapacity(), context->current_cells->data());
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, context->current_cells->size());

  SDL_GL_SwapWindow(context->window);

  u64 end = SDL_GetTicksNS() - start;
  std::println("elapsed: {:7.3f}", end * 1.e-6);

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
  delete context;
  SDL_DestroyWindow(context->window);
  SDL_Quit();
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

  context->view = math::mat4::Identity()
    .translate({camera.offset.x, camera.offset.y, 0})
    .scale({camera.zoom, camera.zoom, 1})
    .translate({-camera.target.x, -camera.target.y, 0});
  context->program.set("projection", context->projection * context->view);
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
  context->projection = math::mat4::ortho(
      0, width,
      0, height,
      -10, 10
      );
  context->current_width = width;
  context->current_height = height;
  context->camera.offset = math::vec2(width, height) / 2.f;
  updateCamera(context);
  SDL_GetWindowSizeInPixels(context->window, &width, &height);
  glViewport(0, 0, width, height);
}

void toggleFullScreen(GContext* context)
{
  u32 flags = SDL_GetWindowFlags(context->window);

  SDL_SetWindowFullscreen(context->window, !(flags & SDL_WINDOW_FULLSCREEN));
}

std::string_view GContext::VERTEX_SHADER = R"(#version 410 core

layout (location = 0) in float aColor;
layout (location = 1) in vec2  aTranslation;

uniform mat4 projection;
uniform vec2 gridSize;
uniform float cellSide;

out vec4 OutColor;

const vec2 VertexPositions[4] = vec2[4](
  vec2(0.0, 0.0),
  vec2(0.8, 0.0),
  vec2(0.8, 0.8),
  vec2(0.0, 0.8)
);

const vec4 palette[22] = vec4[22](
  vec4(0.0, 0.100, 0.800, 1),
  vec4(0.0, 0.095, 0.760, 1),
  vec4(0.0, 0.090, 0.720, 1),
  vec4(0.0, 0.085, 0.680, 1),
  vec4(0.0, 0.080, 0.640, 1),
  vec4(0.0, 0.075, 0.600, 1),
  vec4(0.0, 0.070, 0.560, 1),
  vec4(0.0, 0.065, 0.520, 1),
  vec4(0.0, 0.060, 0.480, 1),
  vec4(0.0, 0.055, 0.440, 1),
  vec4(0.0, 0.050, 0.400, 1),
  vec4(0.0, 0.045, 0.360, 1),
  vec4(0.0, 0.040, 0.320, 1),
  vec4(0.0, 0.035, 0.280, 1),
  vec4(0.0, 0.030, 0.240, 1),
  vec4(0.0, 0.025, 0.200, 1),
  vec4(0.0, 0.020, 0.160, 1),
  vec4(0.0, 0.015, 0.120, 1),
  vec4(0.0, 0.010, 0.080, 1),
  vec4(0.0, 0.005, 0.040, 1),
  vec4(0, 0.0125, 0.1, 1   ),
  vec4(1, 1, 1, 1          )
);

void main()
{
  int i = int(aColor);
  OutColor = palette[i + 20];

  vec2 position = (VertexPositions[gl_VertexID] * cellSide + aTranslation);
  gl_Position = projection * vec4(position, 0, 1);
}
)";

std::string_view GContext::FRAGMENT_SHADER = R"(#version 410 core

in vec4 OutColor;

out vec4 FragColor;

void main()
{
FragColor = OutColor;
}
)";
