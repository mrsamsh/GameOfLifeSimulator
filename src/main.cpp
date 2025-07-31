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
#include <print>

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
  static constexpr i32 Width = 1920 * 2, Height = 1080 * 2;
  static constexpr i32 cellside = 1;
  static constexpr i32 gridWidth = Width / cellside;
  static constexpr i32 gridHeight = Height / cellside;
  static constexpr bool high_dpi = true;
  f32 current_width, current_height;
  math::mat4 projection;
  math::mat4 currentProjection;
  f32 pixel_density;
  struct Camera {
    f32 zoom = 1;
    math::vec2 target = math::vec2(Width, Height) / 2.f;
  } camera;
  SDL_Window* window;
  ShaderProgram program;
  u32 VAO, VBO;
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

void updateCamera(GContext* cam);
void handleResize(GContext* context);

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
      GContext::Width, GContext::Height,
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

  context->projection = math::mat4::ortho(0, GContext::Width, 0, GContext::Height, -10, 10);
  context->currentProjection = context->projection;
  context->program.use().set("projection", context->projection);
  context->program.use().set("gridSize", math::vec2(GContext::gridWidth, GContext::gridHeight));

  glGenVertexArrays(1, &context->VAO);
  glBindVertexArray(context->VAO);
  glGenBuffers(1, &context->VBO);
  glBindBuffer(GL_ARRAY_BUFFER, context->VBO);
  glBufferData(GL_ARRAY_BUFFER, context->current_cells->ByteCapacity(), nullptr, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_BYTE, false, sizeof(i8), 0);
  glVertexAttribDivisor(0, 1);

  glClearColor(0.0, 0.025, 0.2, 1);
  handleResize(context);
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
    const int gridWidth  = GContext::Width  / context->cellside;
    const int gridHeight = GContext::Height / context->cellside;
    std::fill(next_cells.begin(), next_cells.end(), 0);
    for (auto y = 0; y < gridHeight - 1; ++y) {
      for (auto x = 0; x < gridWidth; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex + gridWidth] += value;
      }
    }
    for (auto y = 1; y < gridHeight; ++y) {
      for (auto x = 0; x < gridWidth; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex - gridWidth] += value;
      }
    }
    for (auto y = 0; y < gridHeight; ++y) {
      for (auto x = 0; x < gridWidth - 1; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex + 1] += value;
      }
    }
    for (auto y = 0; y < gridHeight; ++y) {
      for (auto x = 1; x < gridWidth; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex - 1] += value;
      }
    }
    for (auto y = 1; y < gridHeight; ++y) {
      for (auto x = 1; x < gridWidth; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex - 1 - gridWidth] += value;
      }
    }
    for (auto y = 0; y < gridHeight - 1; ++y) {
      for (auto x = 1; x < gridWidth; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex - 1 + gridWidth] += value;
      }
    }
    for (auto y = 0; y < gridHeight - 1; ++y) {
      for (auto x = 0; x < gridWidth - 1; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex + 1 + gridWidth] += value;
      }
    }
    for (auto y = 1; y < gridHeight; ++y) {
      for (auto x = 0; x < gridWidth - 1; ++x) {
        auto cellIndex = x + y * gridWidth;
        auto value = current_cells[cellIndex] == 1 ? 1 : 0;
        next_cells[cellIndex + 1 - gridWidth] += value;
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
    for (usz i = 0; i < current_cells.size(); ++i) {
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

  auto mouseToNormal = context->projection;
  math::vec3 mousepos {};
  auto state = SDL_GetMouseState(&mousepos.x, &mousepos.y);
  static bool last_time, this_time;
  last_time = this_time;
  this_time = (state & SDL_BUTTON_LMASK) != 0;
  mousepos = mouseToNormal.transform(mousepos);
  mousepos = context->currentProjection.inverse().transform(mousepos);
  if (this_time && !last_time) {
    u32 i = floor(mousepos.x) + floor(mousepos.y) * GContext::Width;
    auto& clicked_cell = (*context->current_cells)[i];
    clicked_cell = clicked_cell == 1 ? 0 : 1;
  }

  f32 zoomF = 0;
  math::vec2 camVel;
  static constexpr f64 FPS = 15.0;
  static constexpr f32 CamSpeed = 50;
  static constexpr math::Time Delta = math::seconds(1.0 / FPS);
  if (keyboard[SDL_SCANCODE_K]) zoomF += 2;
  if (keyboard[SDL_SCANCODE_J]) zoomF -= 2;
  if (keyboard[SDL_SCANCODE_D]) camVel.x += 1;
  if (keyboard[SDL_SCANCODE_A]) camVel.x -= 1;
  if (keyboard[SDL_SCANCODE_S]) camVel.y += 1;
  if (keyboard[SDL_SCANCODE_W]) camVel.y -= 1;
  if (zoomF != 0 || !math::isZero(camVel)) {
    context->camera.zoom *= (1 + zoomF * context->pixel_density * Delta.asMilliseconds() / 4000.f);
    context->camera.target += camVel * context->pixel_density * CamSpeed * Delta.asSeconds();
    updateCamera(context);
  }

  // update here
  if (updating)
    calculateNext(*context->current_cells, *context->next_cells);
  // -----------------------------------
  glClear(GL_COLOR_BUFFER_BIT);

  glBindVertexArray(context->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, context->VBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, context->current_cells->ByteCapacity(), context->current_cells->data());
  glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, context->current_cells->size());

  SDL_GL_SwapWindow(context->window);
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
  context->camera.zoom = math::clamp(context->camera.zoom, 1.f, 30.f);

  math::vec2 orig_size = math::vec2(GContext::Width, GContext::Height);// * context->pixel_density;
  math::vec2 orig_half_size = orig_size / 2.f;
  math::vec2 current_size {context->current_width, context->current_height};
  math::vec2 view_size = current_size / context->camera.zoom;
  math::vec2 view_half_size = view_size / 2.f;

  math::vec2 view_half_size_in_world = view_size / (context->camera.zoom * 2.f);

  context->camera.target.x = std::clamp(context->camera.target.x, view_half_size_in_world.x, orig_size.x - view_half_size_in_world.x);
  context->camera.target.y = std::clamp(context->camera.target.y, view_half_size_in_world.y, orig_size.y - view_half_size_in_world.y);

  math::vec2 target = (context->camera.target - orig_half_size) * context->camera.zoom + orig_half_size;
  target -= view_half_size;
  math::mat4 projection = math::mat4::ortho(
      context->camera.target.x - view_half_size_in_world.x,
      context->camera.target.x + view_half_size_in_world.x,
      context->camera.target.y - view_half_size_in_world.y,
      context->camera.target.y + view_half_size_in_world.y,
      -10, 10
      );

  context->program.set("projection", projection);
  context->currentProjection = projection;

  // glViewport(target.x, target.y, view_size.x, view_size.y);
}

void reset_cells(GContext::array_t& cells, unsigned int seed) {

  srand(seed);
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
  SDL_GetWindowSizeInPixels(context->window, &width, &height);
  glViewport(0, 0, width, height);
  context->current_width = width;
  context->current_height = height;
  updateCamera(context);
}

std::string_view GContext::VERTEX_SHADER = R"(#version 410 core

layout (location = 0) in float aColor;

uniform mat4 projection;
uniform vec2 gridSize;

out vec4 OutColor;

const vec2 VertexPositions[4] = vec2[4](
vec2(0.0, 0.0),
vec2(0.8, 0.0),
vec2(0.8, 0.8),
vec2(0.0, 0.8)
);

void main()
{
int i = int(aColor);
switch (i)
{
  case 1:
    OutColor = vec4(1, 1, 1, 1);
    break;
  case 0:
    OutColor = vec4(0, 0.0125, 0.1, 1);
    break;
  default:
    OutColor = vec4(0, 0.1, 0.8, 1) * aColor / -20.0;
    break;
}

vec2 translation;
translation.x = mod(gl_InstanceID, gridSize.x);
translation.y = int(gl_InstanceID) / int(gridSize.x);
vec2 position = VertexPositions[gl_VertexID] + translation;
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
