---
title: "Understanding Vulkan: CommandBuffers"
date: 2022-08-06
categories: vulkan
---
Based on the video from TU Wien on youtube [Commands and Command Buffers](https://www.youtube.com/watch?v=J1A1kWBVrzc&list=PLmIqTlJ6KsE1Jx5HV4sd2jOe3V1KMHHgn&index=4).

The surt is a big *HUH*, there are too many granular instructions to this.

Every single command must be recorded into a `CommandBuffer` and then send that to GPU. By themselves the commands will do nothing.

# Action Commands
They declare computation work that will be done by the GPU.

## Graphics/Drawing Pipeline Commands
`vkCmdDraw`+`something`
`vkCmdClearAttachments`
...

## Compute Pipeline Commands
`vkCmdDispatch`
`vkCmdDispatchBase`
`vkCmdDispatchIndirect`
...

## Transfer Commands
`vkCmdCopy`+`something`
`vkCmdFillBuffer`
`vkcmdBlitImage`
`vkCmdResolveImage`
`vkCmdClearColorImage`
`vkCmdClearDepthStencilImage`
...

## Ray Tracing Pipeline Commands
`vkTraceRaysKHR`
`vkTraceRaysIndirectKHR`
...

## Ray Tracing Acceleration Structure Build Commands
`vkCmdBuildAccelerationStructuresKHR`
`vkCmdBuildAccelerationStructuresIndirectKHR`
...

# State Commands
They do not declare computation, they set the state for the computation to take place as intended.

## Bind Commands
`vkCmdBindDescriptorSets`
`vkCmdBindPipeline`
`vkCmdBindVertexBuffers`
`vkCmdBindIndexBuffers`

## Other Commands . . .
`vkCmdPushConstants`
`vkCmdPushDescriptorSetsKHR`
`vkCmdSetScissor`
`vkCmdSetViewport`
`vkCmdSetDepthBias`
...

# Recording Command Buffers
```c++
VkDevice dvc = /*your logical device*/;
VkQueue q = /* your queue */;

VkCommandPoolCreateInfo cmd_pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
cmd_pool_info.flags = 0;
cmd_pool_info.queueFamilyIndex = 0; // Same index as the queue you want to use for each command
// It is the duty of the commands to know the queues they will be sent to and that the type correspond

VkCommandPool cmd_pool;
vkCreateCommandPool(dvc, &cmd_pool_info, nullptr, &cmd_pool);

VkCommandBufferAllocateInfo cmd_alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
cmd_alloc_info.commandPool = cmd_pool;
cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // ??
cmd_alloc_info.commandBufferCount = 1; // Depend on the number of commands sent

VkCommandBuffer cmd_buf;
vkAllocateCommandBuffers(dvc, &cmd_alloc_info, &cmd_buf);

VkCommandBufferBeginInfo buf_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
buf_begin_info.flags = 0;
vkBeginCommandBuffer(cmd_buf, &buf_begin_info);
//... go to cmd_buffers, shaders, environment, materilas, view, ...
vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,...);
vkCmdDraw(cmd_buf, ...);
//... continue to next cmd buffers, shaders, materials,...
vkEndCommandBuffer(cmd_buf); 
```
Very similar to the immediate mode of OpenGL or ImGUI, Begin -> Declare State -> Record/Do Things -> End

Recording the commandbuffers comes before sending them for submission to the GPU!!!!

```c++
//example loop
while(!command_buffers.empty()) {
    //...
    VkSubmitInfo cmd_submit_info = {};
    cmd_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    cmd_submit_info.commandBufferCout = 1; // ...
    cmd_submit_info.pCommandBuffers = command_buffers[i].data();
    vkQueueSubmit(queue, 1, &cmd_submit_info, VK_NULL_HANDLE); // Now the GPU will send into the internal Queue and use it when possibly
    //...
}
```

The objective of this abstraction is to reduce the amount of things that have to be setup repeatedly when sending commands to the GPU.

## Single Use Submit
```c++
//...
cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
// Transient meaning that is short lived, aka will be used for not much time and can be reset or freed rapidly (Signal possible Driver Optimization)
//...
cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//...
// Move Command Buffer allocation and binding into the loop as each invocation of the queue is intended to be different
while (/*...*/) {
    // Allocate Command Buffer
    // Record commands
    /// Bind Data  + Draw Calls
    // End Record Commands
    // Submit Queue to GPU
}
```

## Reset and Re-Record
```c++
//...
cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
// Allows any commandbuffer to be reset to initial state -> Less allocations but same binding
//...
cmd_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//...
// Allocate Buffer out of the submission loop
//...
while (/*...*/) {
    // Bind, Record and whatever
    // Submit
}
```
### Lifecycle of a CommandBuffer
1. Allocate -> Initial
2. Begin -> Recording
3. End -> Executable
4. Submitted -> Pending
5. Executed -> Invalid !!!!

In order to reuse a commandbuffer, that must have been executed, else we will be overwriting it and have synchronization issues

## CleanUp... Read Documentation

# Secondary Command Buffers
Basically you can setup Command Buffers in multiple threads but you can only submit them from one?
That is using `vkCmdExecuteCommands`...

Check `Render Passes and secondary Command Buffers` by Samsung Developers

# Sending Data to the Command Buffers

## Descritors -> Previous Chapter
## Push Constants
Basically like uniforms but for evey call in the command entire command buffer
`vkCmdPushConstant`
```c
layout(push_constant) uniform PushConstants{
    vec4 color;
    mat 4 mat;
}
```
Just make sure data in shader matches data size passed in passes

## Parameters
What each command requires or needs to be executed properly, not much to say, each command will say it.

## Attributes
Only for Graphics Pipelines, not including Mesh Shaders!
Similar to traditional states in OpenGL:
### Vertex Processing
 - VERTEX_INPUT
 - VERTEX_SHADER
```c++
VkVertexInputBindingDescription binding0 = {};
binding0.binding = 0; // Handle to use, regarding the VertexBuffer to specifically with `vkCmdBindVertexBuffers(...)` which take a buffer to use, and offset at which it starts
binding0.stride = sizeof(float)*3; // Total size of the value ex: vec3
binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 1 Value of the buffer per vertex

// Descriptor for the data in the binding
vkVertexInputAttributeDescription attr0 = {};
attr0.location = 0;
attr0.binding = 0;
attr0.format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 each with 32bits
attr0.offset = 0; // Basically the 

// Bundle all vertex buffer bindings into the state which will be sent
VkPipelineVertexInputStateCreateInfo vtx_instate = {};
vtx_instate.sType = VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vtx_instate.vertexBindingDescriptionCount = 1; // How many bindings
vtx_instate.pVertexBindingDescriptions = &binding0;// Pointer to array of BindingDescriptions
vtx_instate.vertexAttributeDescriptionCount = 1; // How many total attributes
vtx_instate.pVertexAttributeDescriptions = &attribute0; // Pointer to array of AttributeDescriptions

vkGraphicsPipelineCreateInfo gPPL_info = {};
gPPL_info.sType = VK_STRUCTURE_TYPE_GRAPHCIS_PIPELINE_CREATE_INFO;
gPPL_info.pVertexInputState = &vtx_instate;
```
```c
layout (location = 0) in vec3 inPosition;

void main() {
    gl_Position = vec4(inPosition, 1.0);
}
```
#### More than single value
```c
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
//...
```
```c++
VkVertexInputBindingDescription binding0 = {};
binding0.binding = 0; // Handle to use, regarding the VertexBuffer to specifically with `vkCmdBindVertexBuffers(...)` which take a buffer to use, and offset at which it starts
binding0.stride = sizeof(float)*8; // Our struct has 8 float values
binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 1 Value of the buffer per vertex

// Descriptor for the data0 = position in the binding
vkVertexInputAttributeDescription attr0_pos = {};
attr0_pos.location = 0;
attr0_pos.binding = 0;
attr0_pos.format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 each with 32bits
attr0_pos.offset = 0; // Where in the value the data can be read

// Descriptor for the data1 = normal in the binding
vkVertexInputAttributeDescription attr1_normal = {};
attr1_normal.location = 1; // Location matches next position!
attr1_normal.binding = 0;
attr1_normal.format = VK_FORMAT_R32G32B32_SFLOAT; // vec3 each with 32bits
attr1_normal.offset = sizeof(float)*3; // Before normal there are 3 floats representing the position

// Descriptor for the data 2 = uv in the binding
vkVertexInputAttributeDescription attr2_uv = {};
attr2_uv.location = 1; // Location matches next position!
attr2_uv.binding = 0;
attr2_uv.format = VK_FORMAT_R32G32_SFLOAT; // vec2 each with 32bits
attr2_uv.offset = sizeof(float)*3*2; // Before normal there are 3 floats representing the position and 3 floats representing normal


// Bundle all vertex buffer bindings into the state which will be sent
VkPipelineVertexInputStateCreateInfo vtx_instate = {};
vtx_instate.sType = VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
vtx_instate.vertexBindingDescriptionCount = 1; // How many bindings
vtx_instate.pVertexBindingDescriptions = &binding0;// Pointer to array of BindingDescriptions
vtx_instate.vertexAttributeDescriptionCount = 1; // How many total attributes
vtx_instate.pVertexAttributeDescriptions = &attribute0; // Pointer to array of AttributeDescriptions

vkGraphicsPipelineCreateInfo gPPL_info = {};
gPPL_info.sType = VK_STRUCTURE_TYPE_GRAPHCIS_PIPELINE_CREATE_INFO;
gPPL_info.pVertexInputState = &vtx_instate;
```

Instead of joining them in the same bound buffer, you could create 3 buffers and link each data to the binding instead of the location. Not better, just a different way.

### Rasterization ?
### Fragment Processing
 - EARLY_FRAGMENT_TESTS
 - FRAGMENT_SHADER
 - LATE_FRAGMENT_TESTS
### Pixel Processing
 - COLOR_ATTACHMENT_OUTPUT

### GLSL Usage Example