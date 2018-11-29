#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class ProjectBuffer {
public:
    VkImage image;
    VkCommandBuffer cmd;
    VkImageView view;
    VkFramebuffer fb;
};

class Project {
public:
    VkDevice device;
    VkQueue queue;
    VkCommandBuffer draw_cmd;
    VkSwapchainKHR swapchain;
    VkExtent2D swapchain_extent;
    std::vector<ProjectBuffer> buffers;
    VkRenderPass render_pass;
};

Project project_init(int w, int h);
void project_loop(Project& project);
