/*
 * Copyright (c) 2018 Robert Swain
 * Copyright (c) 2016 David McFarland
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * project_init and project_loop are from the MIT-licensed:
 * https://github.com/corngood/SDL_vulkan
 */

#include "project.h"

#include <iostream>
#include <cassert>
#include <SDL.h>
#include <SDL_vulkan.h>

Project project_init(int w, int h) {
    Project project{};

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
    SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "1");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return project;
    }
    SDL_Window* window = SDL_CreateWindow("Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_VULKAN);

    VkResult err;

    VkInstance inst;
    {
        uint32_t extension_count = 0;
        const char *extension_names[64];
        extension_names[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;

        const VkApplicationInfo app = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "SDL_vulkan example",
            .apiVersion = VK_MAKE_VERSION(1, 0, 3),
        };

        {
            unsigned c = 64 - extension_count;
            if (!SDL_Vulkan_GetInstanceExtensions(window, &c, &extension_names[extension_count])) {
                std::cerr << "SDL_Vulkan_GetInstanceExtensions failed: " << SDL_GetError() << std::endl;
                exit(1);
            }
            extension_count += c;
        }

        VkInstanceCreateInfo inst_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app,
            .enabledExtensionCount = extension_count,
            .ppEnabledExtensionNames = extension_names,
        };

        err = vkCreateInstance(&inst_info, NULL, &inst);
        assert(!err);
    }

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, inst, &surface)) {
        std::cerr << "SDL_Vulkan_CreateSurface failed: " << SDL_GetError() << std::endl;
        exit(1);
    }

    VkPhysicalDevice gpu;
    {
        uint32_t gpu_count;
        err = vkEnumeratePhysicalDevices(inst, &gpu_count, NULL);
        assert(!err && gpu_count > 0);

        if (gpu_count > 0) {
            VkPhysicalDevice gpus[gpu_count];
            err = vkEnumeratePhysicalDevices(inst, &gpu_count, gpus);
            assert(!err);
            gpu = gpus[0];
        } else {
            gpu = VK_NULL_HANDLE;
        }
    }

    VkCommandPool cmd_pool;
    {
        uint32_t queue_family_index = UINT32_MAX;
        {
            uint32_t queue_count;
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);
            VkQueueFamilyProperties queue_props[queue_count];
            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);
            assert(queue_count >= 1);

            for (uint32_t i = 0; i < queue_count; i++) {
                VkBool32 supports_present;
                vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supports_present);
                if (supports_present && (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                    queue_family_index = i;
                    break;
                }
            }
            assert(queue_family_index != UINT32_MAX);
        }

        uint32_t extension_count = 0;
        const char *extension_names[64];
        extension_count = 0;
        extension_names[extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

        float queue_priorities[1] = {0.0};
        const VkDeviceQueueCreateInfo queueInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = queue_priorities};

        VkDeviceCreateInfo deviceInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueInfo,
            .enabledExtensionCount = extension_count,
            .ppEnabledExtensionNames = (const char *const *)extension_names,
        };

        err = vkCreateDevice(gpu, &deviceInfo, NULL, &project.device);
        assert(!err);

        vkGetDeviceQueue(project.device, queue_family_index, 0, &project.queue);

        const VkCommandPoolCreateInfo cmd_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = queue_family_index,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        };
        err = vkCreateCommandPool(project.device, &cmd_pool_info, NULL, &cmd_pool);
        assert(!err);
    }

    VkFormat           format;
    VkColorSpaceKHR    color_space;
    {
        uint32_t format_count;
        err = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, NULL);
        assert(!err);

        VkSurfaceFormatKHR formats[format_count];
        err = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, formats);
        assert(!err);

        if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            format = VK_FORMAT_B8G8R8A8_SRGB;
        } else {
            assert(format_count >= 1);
            format = formats[0].format;
        }
        color_space = formats[0].colorSpace;
    }

    {
        const VkCommandBufferAllocateInfo cmd = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };
        err = vkAllocateCommandBuffers(project.device, &cmd, &project.draw_cmd);
        assert(!err);
    }

    uint32_t swapchain_image_count;
    {
        VkSurfaceCapabilitiesKHR surf_cap;
        err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surf_cap);
        assert(!err);

        if (surf_cap.currentExtent.width == (uint32_t)-1) {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            project.swapchain_extent.width = (uint32_t)w;
            project.swapchain_extent.height = (uint32_t)h;
        } else {
            project.swapchain_extent = surf_cap.currentExtent;
        }

        swapchain_image_count = surf_cap.minImageCount + 1;
        if ((surf_cap.maxImageCount > 0) && (swapchain_image_count > surf_cap.maxImageCount)) {
            swapchain_image_count = surf_cap.maxImageCount;
        }

        const VkSwapchainCreateInfoKHR swapchainInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,
            .minImageCount = swapchain_image_count,
            .imageFormat = format,
            .imageColorSpace = color_space,
            .imageExtent = project.swapchain_extent,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = surf_cap.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .imageArrayLayers = 1,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .presentMode = VK_PRESENT_MODE_FIFO_KHR,
            .clipped = 1,
        };

        err = vkCreateSwapchainKHR(project.device, &swapchainInfo, NULL, &project.swapchain);
        assert(!err);
    }

    project.buffers.reserve(swapchain_image_count);

    {
        err = vkGetSwapchainImagesKHR(project.device, project.swapchain, &swapchain_image_count, 0);
        assert(!err);
        VkImage swapchain_images[swapchain_image_count];
        err = vkGetSwapchainImagesKHR(project.device, project.swapchain, &swapchain_image_count, swapchain_images);
        assert(!err);
        for (uint32_t i = 0; i < swapchain_image_count; i++)
            project.buffers[i].image = swapchain_images[i];
    }

    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        VkImageViewCreateInfo color_attachment_view = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .format = format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_R,
                .g = VK_COMPONENT_SWIZZLE_G,
                .b = VK_COMPONENT_SWIZZLE_B,
                .a = VK_COMPONENT_SWIZZLE_A,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .image = project.buffers[i].image,
        };

        err = vkCreateImageView(project.device, &color_attachment_view, NULL, &project.buffers[i].view);
        assert(!err);
    }

    const VkAttachmentDescription attachments[1] = {
        [0] = {
            .format = format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
    };
    const VkAttachmentReference color_reference = {
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    const VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_reference,
    };
    const VkRenderPassCreateInfo rp_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };

    err = vkCreateRenderPass(project.device, &rp_info, NULL, &project.render_pass);
    assert(!err);

    for (uint32_t i = 0; i < swapchain_image_count; i++) {
        VkImageView attachments[1] = {
            [0] = project.buffers[i].view,
        };

        const VkFramebufferCreateInfo fb_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = project.render_pass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = project.swapchain_extent.width,
            .height = project.swapchain_extent.height,
            .layers = 1,
        };

        err = vkCreateFramebuffer(project.device, &fb_info, NULL, &project.buffers[i].fb);
        assert(!err);
    }
    return project;
}

void project_loop(Project& project) {
    VkResult err;

    auto t = .0f;

    for (;; t += .1f) {
        {
            SDL_Event event;
            SDL_bool done = SDL_FALSE;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    done = SDL_TRUE;
                    break;
                }
            }
            if (done) break;
        }

        VkSemaphore present_complete_semaphore;
        {
            VkSemaphoreCreateInfo semaphore_create_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };
            err = vkCreateSemaphore(project.device, &semaphore_create_info, NULL, &present_complete_semaphore);
            assert(!err);
        }

        uint32_t current_buffer;
        err = vkAcquireNextImageKHR(project.device, project.swapchain, UINT64_MAX, present_complete_semaphore, (VkFence)0, &current_buffer);
        assert(!err);

        const VkCommandBufferBeginInfo cmd_buf_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };
        const VkClearValue clear_values[1] = {
            [0] = { .color.float32 = { t * .01f, .2f, .2f, .2f } },
        };

        const VkRenderPassBeginInfo rp_begin = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = project.render_pass,
            .framebuffer = project.buffers[current_buffer].fb,
            .renderArea.extent = project.swapchain_extent,
            .clearValueCount = 1,
            .pClearValues = clear_values,
        };

        err = vkBeginCommandBuffer(project.draw_cmd, &cmd_buf_info);
        assert(!err);

        VkImageMemoryBarrier image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
            .image = project.buffers[current_buffer].image,
        };

        vkCmdPipelineBarrier(
            project.draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0, 0, NULL, 0, NULL, 1, &image_memory_barrier);

        vkCmdBeginRenderPass(project.draw_cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdEndRenderPass(project.draw_cmd);

        VkImageMemoryBarrier present_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
            .image = project.buffers[current_buffer].image,
        };

        vkCmdPipelineBarrier(
            project.draw_cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0, 0, NULL, 0, NULL, 1, &present_barrier);

        err = vkEndCommandBuffer(project.draw_cmd);
        assert(!err);

        VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &project.draw_cmd,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &present_complete_semaphore,
            .pWaitDstStageMask = &pipe_stage_flags,
        };

        err = vkQueueSubmit(project.queue, 1, &submit_info, VK_NULL_HANDLE);
        assert(!err);

        VkPresentInfoKHR present = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &project.swapchain,
            .pImageIndices = &current_buffer,
        };
        err = vkQueuePresentKHR(project.queue, &present);
        if (err == VK_SUBOPTIMAL_KHR)
            std::cerr << "warning: suboptimal present" << std::endl;
        else
            assert(!err);

        err = vkQueueWaitIdle(project.queue);
        assert(err == VK_SUCCESS);

        vkDestroySemaphore(project.device, present_complete_semaphore, NULL);
    }
}
