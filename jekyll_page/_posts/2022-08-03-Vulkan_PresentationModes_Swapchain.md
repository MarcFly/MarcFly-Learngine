---
title: "Understanding Vulkan: Presentation Modes and the Swapchain"
date: 2022-08-03
categories: vulkan
---

These notes are based on the TU Wien [Vulkan Lecture Series #2](https://www.youtube.com/watch?v=nSzQcyQTtRY&list=PLmIqTlJ6KsE1Jx5HV4sd2jOe3V1KMHHgn&index=3)

## Basic Application Render loop

The swapchain holds a set of images the application can draw to and the present in the following steps:
1. App ask for swapchain for image to draw on
2. Swapchain provides image
3. App draws on image
4. Apps sends back image to Swapchain

Now the trouble starts, things go concurrent, to represent, numbers will be repeated.

5. The swapchain now has to present the image
5. The app asks again for another image

6. Swapchain provides an image if there is one
7. Assuming the swapchain had images to provide, the App receives one
7. Swapchain continues presenting
8. App Draws
9. App sends back an image to Swapchain
10. If swapchain still has not finished presenting the previous image, it holds the new one

11. App asks for another image
11. If swapchain still has not finished presenting the previous image, decide on how to operate

This decision the swapchain has to make is based on the `PRESENTATION MODE`

## Understanding Screen Presentation

Monitors have a maximum refresh rate, or how fast they can fully draw to the screen. Measured in Hz, the usual is 60Hz (16.6ms) and for competitive gaming 144Hz (6.94ms).

This time is also called the *Vertical Blank* or *Vertical Blanking Period*, the interval of time in which is safe to swap which image the screen should use to present.    

## Presentation Modes
Knowing how the screen refereshes we can define the presentation modes.
Keep in mind also, that in all modes, when a new image to be drawn is not found in time it will reuse the previously used.
### Immediate

The swapchain will drop the current image being drawn, send back the available image and continue the presentation using the info from the next image in the buffer.

In this case the swapchain gives no single fuck about the screen not having finished presenting the previous frame.

Pros:
 - Fast
Cons:
 - Tearing (combination of different images in the same presentation)

### FIFO (First In First Out)

The swapchain will wait for the screen to present a frame to send the next one to present in another blank period.
If the application draws too quickly, it will generate a queue of frames that will always be presented in order, without a maximum.

Pros:
 - Good for pacing and framerate
Cons:
 - Lag - User receives frames which are not the most recent given their input

### FIFO Relaxed

Same as fifo but will not go through the queue when the swapchain is starving for images to present (draw time > blank period)

Pros:
 - Good if frames are not as fast as blank period 
 - Same as FIFO when frames are consistently faster than blank period
Cons:
 - Can create Tearing if the frames are inconsistent, but way less than Immediate

### Mailbox

FIFO but with a queue size of 1. It can present an image and only hold 1 image in the buffer.

Pros:
 - Fast
 - Good for pacing and framerate
 - No tearing
Cons:
 - Wasteful, images that are drawn while there is still in queue are thrown. Bad on power limited devices (mobile, ultraportables,...)


## Code Examples (C++)

### Initialization
```c++
// Ask window manager to get a surface
surface = (VkSurfaceKHR)mfly::win::getGAPISurface(0, (vk::Instance)instance);

swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
swapchain_create_info.surface = surface;
swapchain_create_info.minImageCount = 4;

// Should use vkGetPhysicalDeviceSurfaceFormatsKHR to fins su&pported
swapchain_create_info.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

swapchain_create_info.imageExtent = VkExtent2D{1920, 1080};
swapchain_create_info.imageArrayLayers = 1;
swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
swapchain_create_info.presentMode = VK_PRESENT_MODE_MAILBOX_KHR; // The good one
// There are many more settings, research
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkSwapchainCreateInfoKHR.html
vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);

// Check how many images we actually got and retrieve them

vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr); // Sending no array to fill to get count
swapchainImageHandles = new VkImage[image_count];
vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchainImageHandles);
```

### Send commands to be drawn on the image
```c++
// Get an image to draw on
vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, // Max time to wait for image (usually as much as possible)
                            imageAvailableSemaphore, // Signal when acquired -> Wait on GPU
                            imageAvailableFence, // Signal when acquired -> Wait on CPU
                            &currImageIndex); // Which image to use from the handles of images in the swapchain


// Setup Draw Calls

VkSubmitInfo submit_info = {}; // Sutrct to give to queue that will send things to GPU
submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submit_info.commandBufferCount = 0; // As many calls you need to do
submit_info.pCommandBuffers = nullptr; // Struct that holds the draw calls
submit_info.waitSemaphoreCount = 1; // Wait for ???
submit_info.signalSemaphoreCount = 1; // Number of Semaphores to alert the queue is done
submit_info.pSignalSemaphores = &renderFinishedSemaphore;

vkQueueSubmit(queue, 1, &submit_info, syncCPUwithGPU_fence); // Check fence for syncing
```