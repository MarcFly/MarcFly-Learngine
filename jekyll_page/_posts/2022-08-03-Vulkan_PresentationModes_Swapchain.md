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