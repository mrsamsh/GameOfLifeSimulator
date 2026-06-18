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
#include <print>
#include <cstring>
#include <string_view>
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <QuartzCore/QuartzCore.h>

#include "Array.hpp"
#include "Clock.hpp"

#define RAND_CHANCE 12

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(*(array)))

struct UniformBufferObject
{
  math::mat4 projection;
  math::vec2 gridSize;
  f32 cellSide;
};

struct GContext
{
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
  NSWindow* metal_window;
  id<MTLDevice> metal_device;
  CAMetalLayer* metal_layer;
  id<MTLLibrary> metal_default_library;
  id<MTLRenderPipelineState> pipeline;
  id<CAMetalDrawable> metal_drawable;
  id<MTLBuffer> cells_buffer,
                ubo,
                translations;
  id<MTLCommandQueue> command_queue;
  id<MTLCommandBuffer> command_buffer;
  static constexpr usz cells_buffer_capacity = gridWidth * gridHeight;
  i8 *current_cells,
     *next_cells;
  void swap_cells() {
    i8* temp = current_cells;
    current_cells = next_cells;
    next_cells = temp;
  }
  bool space_state[2] = {}, reset_state[2] = {}, step_state[2] = {};
  u64 start_time;
  u64 frame_counter = 0;
  MTLViewport viewport;
};

void updateCamera(GContext& context);
void handleResize(GContext& context);
void toggleFullScreen(GContext& context);

void reset_cells(i8* cells);

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
  srand(std::time(nullptr));
  static GContext context;
  context.current_cells = new i8[GContext::gridWidth * GContext::gridHeight];
  context.next_cells = new i8[GContext::gridWidth * GContext::gridHeight];
  *appstate = &context;

  SDL_Init(SDL_INIT_VIDEO);
  bool const* keyboard = SDL_GetKeyboardState(nullptr);
  context.metal_device = MTLCreateSystemDefaultDevice();
  context.window = SDL_CreateWindow(
      "Test",
      GContext::WindowWidth, GContext::WindowHeight,
      (GContext::high_dpi ? SDL_WINDOW_HIGH_PIXEL_DENSITY : 0)
      | SDL_WINDOW_METAL
      // | SDL_WINDOW_FULLSCREEN
      | SDL_WINDOW_RESIZABLE
      );
  SDL_PropertiesID props = SDL_GetWindowProperties(context.window);
  context.metal_window = (NSWindow*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
  context.metal_layer = [CAMetalLayer layer];
  context.metal_layer.device = context.metal_device;
  context.metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  context.metal_window.contentView.layer = context.metal_layer;
  context.metal_window.contentView.wantsLayer = YES;
  CGFloat scale = context.metal_window.screen.backingScaleFactor ?: 1.0;
  context.metal_layer.contentsScale = scale;
  printf("%f\n", scale);
  NSView *view = context.metal_window.contentView;
  CGSize drawable_size = CGSizeMake(view.frame.size.width * scale, view.frame.size.height * scale);
  context.metal_layer.drawableSize = drawable_size;


  context.pixel_density = SDL_GetWindowPixelDensity(context.window);
  context.cells_buffer = [context.metal_device newBufferWithLength:sizeof(i8) * GContext::cells_buffer_capacity
                                                       options:MTLResourceStorageModeShared];

  reset_cells(context.current_cells);

  Array<math::vec2, GContext::gridWidth * GContext::gridHeight> translations;
  for (int y = 0; y < GContext::gridHeight; ++y)
    for (int x = 0; x < GContext::gridWidth; ++x)
      translations[x + y * GContext::gridWidth] = {x, y};

  context.translations = [context.metal_device newBufferWithBytes:translations.data() length:translations.ByteCapacity() options:MTLResourceStorageModeShared];

  context.ubo = [context.metal_device newBufferWithLength:sizeof(UniformBufferObject)
                                                  options:MTLResourceStorageModeShared];

  NSURL *libraryURL = [[NSBundle mainBundle] URLForResource:@"MyShader" withExtension:@"metallib"];
  NSError *error = nil;
  context.metal_default_library = [context.metal_device newLibraryWithURL:libraryURL
                                                                    error:&error];
  if (error != nil) NSLog(@"Failed to create Library");

  context.command_queue = [context.metal_device newCommandQueue];
  id<MTLFunction> vertex_shader = [context.metal_default_library newFunctionWithName:@"vertexShader"];
  id<MTLFunction> fragment_shader = [context.metal_default_library newFunctionWithName:@"fragmentShader"];
  MTLRenderPipelineDescriptor *render_pipeline_desc = [MTLRenderPipelineDescriptor new];
  [render_pipeline_desc setLabel:@"Cells Pipeline"];
  [render_pipeline_desc setVertexFunction:vertex_shader];
  [render_pipeline_desc setFragmentFunction:fragment_shader];
  MTLPixelFormat pixel_format = (MTLPixelFormat)context.metal_layer.pixelFormat;
  [[[render_pipeline_desc colorAttachments]objectAtIndexedSubscript:0]setPixelFormat:pixel_format];
  context.pipeline = [context.metal_device newRenderPipelineStateWithDescriptor:render_pipeline_desc
                                                                          error:&error];
  if (error != nil) NSLog(@"Failed to create pipeline");
  [vertex_shader release];
  [fragment_shader release];

  context.matrices.projection = math::mat4::ortho(0, GContext::WindowWidth, 0, GContext::WindowHeight, -10, 10);
  context.matrices.view = math::mat4::Identity();

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
  auto calculateNext = [&context](const i8* const current_cells, i8* const next_cells) {
    auto const gridWidth = GContext::gridWidth;
    auto const gridHeight = GContext::gridHeight;
    // memset(next_cells, 0, sizeof(*next_cells) * GContext::cells_buffer_capacity);

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


    i8* cells_buffer = (i8*)(context.cells_buffer.contents);
    for (usz i = 0; i < gridWidth * gridHeight; ++i) {
      auto cc = current_cells[i];
      i8 sc = next_cells[i];
      next_cells[i] = 0;
      i8& result = cells_buffer[i];
      if (cc == 1) {
        switch (sc) {
        case 2:
        case 3:
          next_cells[i] = 1;
          result = 1;
          break;
        default:
          result = -20;
          break;
        }
      } else {
        if (sc == 3) {
          next_cells[i] = 1;
          result = 1;
        } else {
          result = std::min(0, result + 1);
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
    reset_cells(context.current_cells);
  }

  bool* step = context.step_state;
  step[0] = step[1];
  step[1] = keyboard[SDL_SCANCODE_E];

  if ((step[1] && ! step[0]) || keyboard[SDL_SCANCODE_Q]) {
    updating = false;
    calculateNext(context.current_cells, context.next_cells);
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
    auto& clicked_cell = (context.current_cells)[i];
    clicked_cell = clicked_cell == 1 ? 0 : 1;
    auto& buffer_cell = ((i8*)context.cells_buffer.contents)[i];
    buffer_cell = buffer_cell == 1 ? 0 : 1;
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

  {
  // update here

  // draw here
  // first upload pass

  u64 beginning = SDL_GetTicksNS();
  context.metal_drawable = [context.metal_layer nextDrawable];
  context.command_buffer = [context.command_queue commandBuffer];
  MTLRenderPassDescriptor *render_pass_desc = [MTLRenderPassDescriptor new];

  MTLRenderPassColorAttachmentDescriptor *cd = [[render_pass_desc colorAttachments] objectAtIndexedSubscript:0];
  [cd setTexture:[context.metal_drawable texture]];
  [cd setLoadAction: MTLLoadActionClear];
  [cd setClearColor:MTLClearColorMake(0, 0.025f, 0.2f, 1)];
  [cd setStoreAction:MTLStoreActionStore];
  id<MTLRenderCommandEncoder> render_command_encoder = 
    [context.command_buffer renderCommandEncoderWithDescriptor:render_pass_desc];
  [render_command_encoder setRenderPipelineState:context.pipeline];
  UniformBufferObject* ubo = (UniformBufferObject*)context.ubo.contents;
  ubo->projection = context.matrices.projection * context.matrices.view;
  ubo->gridSize = {GContext::gridWidth, GContext::gridHeight};
  ubo->cellSide = GContext::CellSide;
  [render_command_encoder setVertexBuffer:context.ubo offset:0 atIndex:0];
  [render_command_encoder setVertexBuffer:context.cells_buffer offset:0 atIndex:1];
  [render_command_encoder setVertexBuffer:context.translations offset:0 atIndex:2];

  [render_command_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                             vertexStart:0
                             vertexCount:6
                           instanceCount:context.cells_buffer_capacity];

  [render_command_encoder endEncoding];
  [context.command_buffer presentDrawable:context.metal_drawable];
  [context.command_buffer commit];
  [context.command_buffer waitUntilScheduled];
  [render_pass_desc release];
  NSLog(@"rendering elapsed: %f ms", (SDL_GetTicksNS() - beginning) * 1.e-6);


  beginning = SDL_GetTicksNS();
  if (updating)
  {
    calculateNext(context.current_cells, context.next_cells);
    context.swap_cells();
  }
  NSLog(@"updating elapsed: %f ms", (SDL_GetTicksNS() - beginning) * 1.e-6);

  }

  //   SDL_DelayPrecise((Delta - elapsed).asNanoseconds());

  context.frame_counter++;
  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
  GContext& context = *(GContext*)appstate;
  u64 elapsed = SDL_GetTicksNS() - context.start_time;
  u64 ns_per_frame = elapsed / context.frame_counter;
  std::println("total elapsed   : {:7.3} sec", elapsed * 1.e-9);
  std::println("total frames    : {:3d} frame", context.frame_counter);
  std::println("avg ms per frame: {:7.3f} ms", ns_per_frame * 1.e-6);
  std::println("fps             : {:7.3f}", context.frame_counter / (elapsed * 1.e-9));
  // SDL_ReleaseGPUBuffer(context.device, context.cell_buffer);
  // SDL_ReleaseGPUTransferBuffer(context.device, context.cell_transfer_buffer);
  // SDL_DestroyGPUDevice(context.device);
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

void reset_cells(i8* cells) {
  for (int i = 0; i < GContext::cells_buffer_capacity; ++i) {
    if (rand() % RAND_CHANCE == 3) {
      cells[i] = 1;
    } else {
      cells[i] = 0;
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
  NSView *view = context.metal_window.contentView;
  CGFloat scale = context.metal_window.screen.backingScaleFactor ?: 1.0;
  CGSize drawable_size = CGSizeMake(view.frame.size.width * scale, view.frame.size.height * scale);
  context.metal_layer.drawableSize = drawable_size;
  context.viewport = {0, 0, drawable_size.width, drawable_size.height};
}

void toggleFullScreen(GContext& context)
{
  u32 flags = SDL_GetWindowFlags(context.window);

  SDL_SetWindowFullscreen(context.window, !(flags & SDL_WINDOW_FULLSCREEN));
}

