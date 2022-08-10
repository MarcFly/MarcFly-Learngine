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