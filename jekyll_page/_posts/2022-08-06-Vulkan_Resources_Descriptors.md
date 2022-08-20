---
title: "Understanding Vulkan: Resources and Descriptors"
date: 2022-08-06
categories: vulkan
---

## GPU Memory Model

Shader Execution
 1. CPU Uploads to GPU VRAM
 2. GPU Transfer critical resources to L2 Cache (smaller)
 3. Each cluster of execution units receives part of the critical resources in their respective L1 Caches
 4. Execution Units work on their tasks

## Resources
Views of the memory in VRAM
 - Buffers: Just Plain Old Data, arrays of bytes stored
 - Images: Ararys of bytes with format information, can be multi-dimensional and may have metadata
 - Samplers: Sample from Images at certain coordinates + Interpolate
 - Acceleration Structures: Like Hierarchy Trres used for raytracing

## Descriptors
Describe how to acess, use, where it is located relative to other data, metadata and how to combine it with other resources.

# Buffers
When creating a buffer, you have to allocate a `VkMemory` to GPU and create a `VkBuffer` which represents a handle to that memory allocation.

## Buffer TYPES

### VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
Read Only Data which is loaded down to L1 caches during the execution of a command

### VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
Read/Write Data which is loaded down to L1 caches during the execution of a command. Accepts atomic operations but in the L2 cache. They can also be stored back from shader to VRAM.

### VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER / VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
Optional type of buffer which allows for formatted upload such as representing as vec3/vec/...
*Don't really understand for now

It basically generates a `VkBufferView` which provides info on how to handle the data in the buffer.

### VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC / VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
Pry into a previous memory allocation and set your data there

### VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT
Very VERY small buffer that does not require allocation, it will be popped up there but is very restricted in size.

## Image Types
They almost always require an ImageView

### Storage Image
Load and Store Operations - Atomic
Accessed through pixel coordinates

### Sampled Image
Only Load Operations - Like Uniform Buffers
Sampled through normalized coordinates (0-1)

### Input attachment
Load-only in a single renderpass
Can access only a single coordinate of an image

## Samplers
Usually do not need memory views, they are inlined mostly

## VkAccelerationStrucutreKHR
They reference the buffers but the structure itself will be handled by the graphics card...

# Descriptors
1 Descriptor - 1 Resource
They should be arranged in sets that are used in conjunction -> Reduce Bindsets which reduce the overhead when changing programs to execute

We can have multiple sets per program and in the application

## Bind-full and Bindless
Hearing now about bindsets comes into the issue of accessing uniform values, changing between program executions and most importantly the overhead of doing so. After searching for a bit, I found the following vid on the topic which I will summarize for myself.

[Exploration of Bindless Rendering in Vulkan, DX12 and OpenGL](https://www.youtube.com/watch?v=SVm0HanVTRw)

### Bind-full
Each call requires a binding of data because each call requires different resources
```c++
for view {
    bind view resources // Camera, Environment,...
    for shader {
        bind shader pipeline // Pipeline State, depth, scissor, ...
        bind shader resources // ?
        for material {
            bind material resources // Textures, Parameters
            for object {
                bind object resources // Meshes, skeletons, ...
                DRAW()
            }
        }
    }
}
/// Representing Frequency binding - bind what you need per draw and not repeat
/// Before there was slot based binding -> Access limited slots of the hardware
```
Why? -> Bind is expensive, avoid binding everything per draw call -> minimize bindings

### Bindless
Bind everything at the start
Then shaders have handles to the bound data
```c++
for object {
    populate data
    bind data
    update descriptor array
}
bind everything
for view {
    set view_data index
    {
        for shader {
            bind shader pipeline
            for object {
                set material_index
                set object_index
            }
        }
    }
}
```

It works because of descriptors -> Info about data
Binding makes a descriptor accessible for shaders to access
Every shader needs many descriptors


#### Benefits
 - Flexible Rendering Architecture
 - Less Updates -> Single Big update for data in bulk, easier to optimize
 - Same referral of dat between GPU and CPU - Handles
 - Storing info for future frames easily

 - Fast Vertex Pulling -> No Binding of Vertex buffers
 - Raytracing -> Fast access to data from acceleration structures
 - Modify resource index in shader for other shaders to use
 - Use Material ID instead of values for GBuffer
 - Usage of material is for terrains -> more materials
 - GPU Driven Rendering 

#### Drawbacks
 - Hardware support -> Support from 2013<
 - Memory Indirection -> Cache missing when accessing different sets of data... `TODO: UNDRESTAND DATA ACCESSING IN GPU AND ITS COSTS`
 - No validation of descriptors
 - Inside shader validation -> debug overhead and more code and tests...
 - Harder to debug -> Learn RenderDOC or die
 - App manages descriptors, more code...

#### Uniform vs Non-uniform Indexing
**Uniform:**
 - Static Indices per shader - Compile time
 - Multiple Descriptors to use different indices for some data
 - Per shader, each invocation of it (per pixe/per core/...) will only be able to index the descriptors sent

**Non-Uniform:**
 - Different invocations of a program can sample different indices
 - NECESSARY FOR BINDLESS (can settle for `Dynamically Uniform Indexing`)

#### Unbounded Descriptor Arrays
Generic memory in GPU
Create descriptors Arrays ahead of time then use them

#### Descriptor Updating Synchronization
Manual synchronization of descriptor updates - Natural to Vulkan and DX12
Application needs data race prevention
Not-in-use can be updated freely (Good :)
Vulkan has more requirements...???

### Vulkan Bindless
#### Descriptor Indexing
`VK_EXT_descriptor_indexing` -> Core in 1.2 -> Optional (require it)
Not very available even in recent hardware (2018+ drivers needed)

`NonUniformIndexing` in shaders
`Unbounded Descriptor Arrays`
Less Constraints in descriptor sets updates

#### Updating Descriptor Sets
`vkUpdateDescriptorSets` -> It was always needed
Use the following flags:
`VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT`
 - Allow update of descriptors that are not in-use even if they are bound
`VK_DESCRIPTOR_BINDING_PARTIALLY_BIT`
 - Not need descriptors to be defined/filled
`VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT`
 - Update descriptors that are NOT IN USE by the command buffer that is executing, but can be if not in use and bound

#### Quirks
 - Unbounded Arrays must be the last descriptor in a descriptor set?
 - No Arrays of descriptor type...?
 - Bad for validation...

## Back to the Topic
Create both pipelines to understand them... how annoying.
Not many users will benefit, specially mobile :sadge:, because bindless is not available.

## Summary of Descriptor and Descriptor Sets
 - 1 Descriptor = 1 Data Buffer
 - 1 Descriptor set = infinity descriptors
 - At runtime, commandbuffers are evaluated by data they reference and organized to be submitted in order.

## [VkPipelineBindPoint](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineBindPoint.html)
Binding the shader pipeline to a state
 - `VK_PIPELINE_BIND_POINT_GRAPHICS` -> Graphics - Vertes, Fragment, Tesselation, Geometry,... shaders
 - `VK_PIPELINE_BIND_POINT_COMPUTE` -> Compute Shaders
 - `VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR` -> Ray Tracing ...
 - `VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI`???? -> ????

Basically allows for binding different descriptor sets so that they do not overlap or are duplicated!

## Allocating Descriptor Sets
Allocated in a Descriptor Pool, which is handled opaquely

1. Create Pool knowing which descriptors you need
    - `VkDescriptorPoolSize` per descriptor -> `vkCreateDescriptorPool`
2. Create a Layout per Descriptor set
    - `VkDescriptorSetLayout` -> bind with `VkDescriptorSetLayoutBinding` per resource
3. Allocate new set to the pool
    - `vkAllocateDescriptorSets` -> Remember to reference the `VkDescriptorPool` in the `VkDescriptorSetAllocateInfo`...

Then bind the `VkDescriptorSet` when needed when giving commands to each pipeline with `vkCmdBindDescriptorSets`

## Descriptor Types and Usage in GLSL
Sets depend on the descriptor set index passed to the command
Bindings depend on the position the index of data handles are inside each descriptor set

### `VK_DESCRIPTOR_TYPE_SAMPLER`
Set `VkSamples` in `VkDescriptorImageInfo`
```c
layout (set = 0, binding = 0) uniform sampler s;
//sampledImage
//...
vec4 rgba = texture(sampler2D(sampledImage, s), vec2(0.5,0.5));
```

### `VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE`
Set `VkImageView` and `VkImageLayout`
```c
// sampler s
layout(set = 0, binding = 1) uniform texture2D sampledImage;
//...
vec4 rgba = texture(sampler2D(sampledImage, s), vec2(0.5,0.5));
```

### `VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER`
Does the work of both sampler + image_sampler
Set all members of `VkDescriptorImageInfo`
```c
layout (set = 1, binding = 0) uniform sampler2D combinedImageSampler;
//...
vec4 rgba = texture(combinedImageSampler, vec2(0.5, 0.5));
```

### `VK_DESCRIPTOR_TYPE_STORAGE_IMAGE`
Set `VkImageView` and `VkImageLayout`
```c
layout (set = 2, binding = 0, rgba /*required_format*/) uniform image2D storageImage;
//...
vec4 rgba = imageLoad(storageImage, ivec2(2,2));
imageStore(storeImage, ivec2(2,2), vec4(0.1, 0.2, 0.3, 1.0));
```

### `VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER`
Set `VkWriteDescriptorSet::pTexelBufferView`...?
```c
layout (set = 3, binding = 0) uniform samplerBuffer uniformTexelBuffer;
//...
int index = 0;
vec4 formattedValue = texelFetch(uniformTexelBuffer, index);
```

### `VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER`
Give multiple `VkBufferView` to the descriptor
```c
layout (set = 4, binding = 0, rgba32f /*required_format*/) uniform imageBuffer storageTexelBuffer;
//...
int index = 0;
vec4 formattedValue = imageLoad(storageTexelBuffer, index);
imageStore(storageTexelBuffer, index, vec4(1.0, 2.0, 3.0, 4.0));
```

### `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER` / `VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC` / `VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT`
Set `VkDescriptorBufferInfo` properly
For inlined, `dstArrayElement` + `descriptorCount`
```c
layout (set = 5, binding = 0) uniform UniformBuffer {
    mat4 projection;
    mat4 view;
    mat4 mode;
} uniformBuffer;
//...
mat4 M = uniformBuffer.projection * uniformBuffer.view * uniformBuffer.model;
```

### `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER` / `VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC`
Add buffers through `VkWriteDescriptorSet::pBufferInfo`
```c
layout (set = 5, binding = 0) uniform UniformBuffer {
    float dt;
} uniformBuffer; // For example purposes

struct Particle {
    vec4 pos;
    vec4 vel;
};

layout (set = 5, binding = 1) buffer StorageBuffer {
    Particle particles[];
} storageBuffer;
//...
int i = 0;
vec3 p = storageBuffer.particles[i].pos.xyz;
vec3 v = storageBuffer.particles[i].vel.xyz;
storageBuffer.particles[i].pos.xyz = p + v * uniformBuffer.dt;
```

### `VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT`
Same as Image Descriptors -> Set `VkImageView` and `VkImageLayout`
```c
layout (input_attachment_index = 0, set = 6, binding = 0) uniform subpassInput depthImg;
// Images are passed in bulk here, for example the results of a Geometry Pass
//...
float depthAtFramebufferLocation = subpassLoad(depthImaage).r;
```

### `VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR`
`VkWriteDescriptorSet::pNext` chain, where each contain a pointer to `VkWriteDescriptorSetAccelerationStructureKHR`

```c
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_ray_query : require
layout(set = 7, binding = 0) uniform accelerationStructureEXT topLevelAS;
//...
vec3 og = vec3(0.0,0.0,0.0);
vec3 dir = vec3(0.0,0.0,1.0);
float tMin = 0.01;
float tMax = 1000.0;
traceRayEXT(topLevelAS, gl_RayFlagsNoneEXT, 0xFF, 0,0,0, og, tMin, tMax, 0);
//...
rayQueryEXT rayQuery;
rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsNoneEXT, 0xFF, og, tMin, dir, tMax);
```