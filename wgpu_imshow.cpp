#include "wgpu_imshow.hpp"
#include "slime_mold.hpp"
#include <webgpu/webgpu.h>

namespace {

struct Config {
//  static constexpr auto image_format = WGPUTextureFormat_RGBA32Float;
  static constexpr auto image_format = WGPUTextureFormat_RGBA8Unorm;
  static constexpr int texture_dim = gen::SlimeMoldConfig::texture_dim;
  static constexpr int bytes_per_component = 1; //  u8 components
  static constexpr int num_components_per_pixel = 4;  //  rgb
};

struct Uniforms {
  float enable_bw_viewport_width_height_full_screen[4];
};

struct {
  bool prepared{};
  bool initialized{};
  WGPURenderPipeline pipeline{};
  WGPUPipelineLayout pipeline_layout{};
  WGPUBindGroupLayout pipeline_bind_group_layout{};
  WGPUBindGroup bind_group{};

  WGPUTexture image{};
  WGPUTextureView image_view{};
  WGPUSampler image_sampler{};
  WGPUBuffer uniform_buffer{};
  Uniforms uniforms{};
} globals;

const char* get_source() {
  const char* source = R"(

  struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) uv: vec2f
  };

  struct Uniforms {
    enable_bw_viewport_width_height_full_screen: vec4f
  };

  @group(0) @binding(0) var my_texture: texture_2d<f32>;
  @group(0) @binding(1) var my_texture_sampler: sampler;
  @group(0) @binding(2) var<uniform> my_uniforms: Uniforms;

  @vertex
  fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
    var out: VertexOutput;
    var p = vec2f(0.0, 0.0);

    if (in_vertex_index == 0u) {
      p = vec2f(-1.0, -1.0);
    } else if (in_vertex_index == 1u) {
      p = vec2f(-1.0, 3.0);
    } else {
      p = vec2f(3.0, -1.0);
    }

    out.uv = p * 0.5 + 0.5;
    out.position = vec4f(p, 0.0, 1.0);
    return out;
  }

  @fragment
  fn fs_main(in: VertexOutput) -> @location(0) vec4f {
//    let uv = vec2i(in.uv * vec2f(256, 256));
//    let color = vec3f(in.uv, 1.0f);
//    let color = textureLoad(my_texture, uv, 0).rgb;

    let viewport_dims = my_uniforms.enable_bw_viewport_width_height_full_screen.yz;
    let abs_pixel = floor(in.uv * viewport_dims);
    let adj_uv = (abs_pixel + vec2f(0.5f)) / vec2f(768.0f); //  tweak to set image size
    let adj_mask = f32(
      adj_uv.x <= 1.0f && adj_uv.y <= 1.0f && adj_uv.x >= 0.0f && adj_uv.y >= 0.0f);

    let use_fs = my_uniforms.enable_bw_viewport_width_height_full_screen.w > 0.0f;
    let pixel_mask = select(adj_mask, 1.0f, use_fs);
    let use_uv = select(adj_uv, in.uv, use_fs);

    let color = textureSample(my_texture, my_texture_sampler, use_uv).rgb;
    let mu = (color.r + color.g + color.b) / 3.0f;
    let color_mean = vec4f(mu, mu, mu, 1.0f);

    let enable_bw = my_uniforms.enable_bw_viewport_width_height_full_screen.x > 0.0f;
    if (enable_bw) {
      return vec4f(vec3f(mu) * pixel_mask, 1.0f);
    } else {
      return vec4f(color * pixel_mask, 1.0);
    }
  }

  )";
  return source;
}

WGPUBindGroupLayout create_bind_group_layout(WGPUDevice device) {
  // the texture binding
  WGPUBindGroupLayoutEntry binds[3]{};

  auto& tex_bind_layout = binds[0];
  tex_bind_layout.binding = 0;
  tex_bind_layout.visibility = WGPUShaderStage_Fragment;
  tex_bind_layout.texture.sampleType = WGPUTextureSampleType_Float;
  tex_bind_layout.texture.viewDimension = WGPUTextureViewDimension_2D;

  auto& tex_sampler_layout = binds[1];
  tex_sampler_layout.binding = 1;
  tex_sampler_layout.visibility = WGPUShaderStage_Fragment;
  tex_sampler_layout.sampler.type = WGPUSamplerBindingType_Filtering;

  auto& uniform_buffer_layout = binds[2];
  uniform_buffer_layout.binding = 2;
  uniform_buffer_layout.visibility = WGPUShaderStage_Fragment;
  uniform_buffer_layout.buffer.type = WGPUBufferBindingType_Uniform;
  uniform_buffer_layout.buffer.minBindingSize = sizeof(Uniforms);
  uniform_buffer_layout.buffer.hasDynamicOffset = false;

  // create a bind group layout
  WGPUBindGroupLayoutDescriptor bg_layout_desc{};
  bg_layout_desc.nextInChain = nullptr;
  bg_layout_desc.entryCount = 3;
  bg_layout_desc.entries = binds;
  WGPUBindGroupLayout bg_layout = wgpuDeviceCreateBindGroupLayout(device, &bg_layout_desc);
  return bg_layout;
}

WGPUBindGroup create_bind_group(
  WGPUDevice device, WGPUBindGroupLayout bg_layout,
  WGPUTextureView image, WGPUSampler image_sampler, WGPUBuffer uniform_buffer) {
  //
  WGPUBindGroupEntry bindings[3]{};

  auto& im_binding = bindings[0];
  im_binding.binding = 0;
  im_binding.textureView = image;

  auto& sampler_binding = bindings[1];
  sampler_binding.binding = 1;
  sampler_binding.sampler = image_sampler;

  auto& buff_binding = bindings[2];
  buff_binding.binding = 2;
  buff_binding.buffer = uniform_buffer;
  buff_binding.size = sizeof(Uniforms);

  WGPUBindGroupDescriptor bind_group_desc{};
  bind_group_desc.layout = bg_layout;
  bind_group_desc.entryCount = 3;
  bind_group_desc.entries = bindings;
  WGPUBindGroup bind_group = wgpuDeviceCreateBindGroup(device, &bind_group_desc);
  return bind_group;
}

bool create_uniform_buffer(WGPUDevice device) {
  WGPUBufferDescriptor buff_desc{};
  buff_desc.size = sizeof(Uniforms);
  buff_desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
  auto buff = wgpuDeviceCreateBuffer(device, &buff_desc);
  if (!buff) {
    return false;
  } else {
    globals.uniform_buffer = buff;
    return true;
  }
}

void create_image(WGPUDevice device) {
  WGPUTextureDescriptor texture_desc{};
  texture_desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
  texture_desc.dimension = WGPUTextureDimension_2D;
  texture_desc.size = { Config::texture_dim, Config::texture_dim, 1 };
  texture_desc.format = Config::image_format;
  texture_desc.mipLevelCount = 1;
  texture_desc.sampleCount = 1;
  texture_desc.viewFormatCount = 0;
  texture_desc.viewFormats = nullptr;

  WGPUTexture texture = wgpuDeviceCreateTexture(device, &texture_desc);
  globals.image = texture;

  WGPUTextureViewDescriptor texture_view_desc{};
  texture_view_desc.aspect = WGPUTextureAspect_All;
  texture_view_desc.baseArrayLayer = 0;
  texture_view_desc.arrayLayerCount = 1;
  texture_view_desc.baseMipLevel = 0;
  texture_view_desc.mipLevelCount = 1;
  texture_view_desc.dimension = WGPUTextureViewDimension_2D;
  texture_view_desc.format = texture_desc.format;
  WGPUTextureView texture_view = wgpuTextureCreateView(texture, &texture_view_desc);
  globals.image_view = texture_view;

  WGPUSamplerDescriptor sampler_desc{};
  sampler_desc.minFilter = WGPUFilterMode_Linear;
  sampler_desc.magFilter = WGPUFilterMode_Linear;
  globals.image_sampler = wgpuDeviceCreateSampler(device, &sampler_desc);

  //  @TODO
//  wgpuTextureDestroy(texture);
//  wgpuTextureRelease(texture);
}

bool create_pipeline(const wgpu::Context& context) {
  auto device = (WGPUDevice) context.wgpu_device;

  WGPURenderPipelineDescriptor pipe_desc{};

  //  vert
  WGPUShaderModuleDescriptor vert_shader_desc{};
  WGPUShaderModuleWGSLDescriptor vert_shader_code_desc{};
  vert_shader_desc.nextInChain = &vert_shader_code_desc.chain;

  vert_shader_code_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  vert_shader_code_desc.code = get_source();
  WGPUShaderModule vert_sm = wgpuDeviceCreateShaderModule(device, &vert_shader_desc);

  //  frag
  WGPUShaderModuleDescriptor frag_shader_desc{};
  WGPUShaderModuleWGSLDescriptor frag_shader_code_desc{};
  frag_shader_desc.nextInChain = &frag_shader_code_desc.chain;

  frag_shader_code_desc.chain.sType = WGPUSType_ShaderModuleWGSLDescriptor;
  frag_shader_code_desc.code = get_source();
  WGPUShaderModule frag_sm = wgpuDeviceCreateShaderModule(device, &frag_shader_desc);

  pipe_desc.vertex.module = vert_sm;
  pipe_desc.vertex.entryPoint = "vs_main";
  pipe_desc.vertex.constantCount = 0;
  pipe_desc.vertex.constants = nullptr;

  pipe_desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
  pipe_desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
  pipe_desc.primitive.frontFace = WGPUFrontFace_CCW;
  pipe_desc.primitive.cullMode = WGPUCullMode_None;

  WGPUFragmentState frag_state{};
  pipe_desc.fragment = &frag_state;
  frag_state.module = frag_sm;
  frag_state.entryPoint = "fs_main";
  frag_state.constantCount = 0;
  frag_state.constants = nullptr;

  WGPUBlendState blend_state{};
  blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
  blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
  blend_state.color.operation = WGPUBlendOperation_Add;
  blend_state.alpha.srcFactor = WGPUBlendFactor_Zero;
  blend_state.alpha.dstFactor = WGPUBlendFactor_One;
  blend_state.alpha.operation = WGPUBlendOperation_Add;

  pipe_desc.multisample.count = 1;
  pipe_desc.multisample.mask = ~0u;
  pipe_desc.multisample.alphaToCoverageEnabled = false;

  WGPUColorTargetState colorTarget{};
  colorTarget.format = (WGPUTextureFormat) context.surface_format;
  colorTarget.blend = &blend_state;
  colorTarget.writeMask = WGPUColorWriteMask_All; // We could write to only some of the color channels.

  frag_state.targetCount = 1;
  frag_state.targets = &colorTarget;

  WGPUBindGroupLayout bg_layout = create_bind_group_layout(device);
  globals.pipeline_bind_group_layout = bg_layout;

  WGPUPipelineLayoutDescriptor pipe_layout_desc{};
  pipe_layout_desc.bindGroupLayouts = &bg_layout;
  pipe_layout_desc.bindGroupLayoutCount = 1;
  WGPUPipelineLayout pipe_layout = wgpuDeviceCreatePipelineLayout(device, &pipe_layout_desc);
  globals.pipeline_layout = pipe_layout;

  pipe_desc.layout = pipe_layout;

  WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &pipe_desc);
  globals.pipeline = pipeline;

  return true;
}

bool try_initialize(const wgpu::Context& context) {
  if (!create_pipeline(context)) {
    return false;
  }

  if (!globals.image) {
    create_image((WGPUDevice) context.wgpu_device);
  }

  if (!globals.uniform_buffer) {
    if (!create_uniform_buffer((WGPUDevice) context.wgpu_device)) {
      return false;
    }
  }

  if (!globals.bind_group) {
    globals.bind_group = create_bind_group(
      (WGPUDevice) context.wgpu_device,
      globals.pipeline_bind_group_layout, globals.image_view,
      globals.image_sampler, globals.uniform_buffer);
  }

  return true;
}

} //  anon

void wgpu::begin_frame(const Context& context, const void* image_data) {
  globals.prepared = false;

  if (!globals.initialized) {
    if (!try_initialize(context)) {
      return;
    } else {
      globals.initialized = true;
    }
  }

  {
    //  image
    WGPUImageCopyTexture dst{};
    dst.aspect = WGPUTextureAspect_All;
    dst.mipLevel = 0;
    dst.texture = globals.image;

    WGPUTextureDataLayout src_layout{};
    src_layout.bytesPerRow =
      Config::texture_dim * Config::bytes_per_component * Config::num_components_per_pixel;
    src_layout.rowsPerImage = Config::texture_dim;
    const int tot_size = int(src_layout.bytesPerRow * src_layout.rowsPerImage);

    WGPUExtent3D write_size{};
    write_size.depthOrArrayLayers = 1;
    write_size.width = Config::texture_dim;
    write_size.height = Config::texture_dim;

    auto device = (WGPUDevice) context.wgpu_device;
    wgpuQueueWriteTexture(
      wgpuDeviceGetQueue(device), &dst, image_data, tot_size, &src_layout, &write_size);
  }
  {
    //  uniform buffer
    globals.uniforms.enable_bw_viewport_width_height_full_screen[0] = float(context.enable_bw);
    globals.uniforms.enable_bw_viewport_width_height_full_screen[1] = float(context.viewport_width);
    globals.uniforms.enable_bw_viewport_width_height_full_screen[2] = float(context.viewport_height);
    globals.uniforms.enable_bw_viewport_width_height_full_screen[3] = float(context.full_screen);

    auto device = (WGPUDevice) context.wgpu_device;
    wgpuQueueWriteBuffer(
      wgpuDeviceGetQueue(device), globals.uniform_buffer, 0,
      &globals.uniforms, sizeof(Uniforms));
  }

  globals.prepared = true;
}

void wgpu::draw_image(void* rp) {
  if (!globals.prepared) {
    return;
  }

  auto render_pass = (WGPURenderPassEncoder) rp;
  wgpuRenderPassEncoderSetPipeline(render_pass, globals.pipeline);
  wgpuRenderPassEncoderSetBindGroup(render_pass, 0, globals.bind_group, 0, nullptr);
  wgpuRenderPassEncoderDraw(render_pass, 3, 1, 0, 0);
}