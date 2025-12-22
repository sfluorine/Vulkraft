#pragma once

#include <fmt/base.h>

#include <cstdlib>
#include <unordered_set>
#include <vector>

#include "vulkan.h"

#define VKH_SHADER_STAGE_VERTEX 0
#define VKH_SHADER_STAGE_FRAGMENT 1
#define VKH_SHADER_STAGE_COUNT 2

#define VK_CHECK(__EXPR__)                                                                                                  \
    do {                                                                                                                    \
        auto __result__ = (__EXPR__);                                                                                       \
        if (__result__ != VK_SUCCESS) {                                                                                     \
            fmt::print(stderr, "{}:{}: {} returned {}\n", __FILE__, __LINE__, #__EXPR__, static_cast<int32_t>(__result__)); \
            abort();                                                                                                        \
        }                                                                                                                   \
    } while (0)

struct VKHVertexLayout {
    std::vector<VkVertexInputBindingDescription> binding_descs;
    std::vector<VkVertexInputAttributeDescription> attribute_descs;
};

class VKHVertexLayoutBuilder {
public:
    VKHVertexLayoutBuilder() = default;

    VKHVertexLayoutBuilder& push_binding(
        uint32_t binding,
        uint32_t stride,
        VkVertexInputRate input_rate);

    VKHVertexLayoutBuilder& push_attribute(
        uint32_t binding,
        uint32_t location,
        uint32_t offset,
        VkFormat format);

    VKHVertexLayout build();

private:
    std::vector<VkVertexInputBindingDescription> m_binding_descs;
    std::vector<VkVertexInputAttributeDescription> m_attribute_descs;
};

class VKHGraphicsPipelineBuilder {
public:
    explicit VKHGraphicsPipelineBuilder(VkDevice device);

    VKHGraphicsPipelineBuilder& set_rendering_format(
        VkFormat color_format,
        VkFormat depth_format = VK_FORMAT_UNDEFINED,
        VkFormat stencil_format = VK_FORMAT_UNDEFINED);

    VKHGraphicsPipelineBuilder& set_vertex_and_fragment(
        VkShaderModule vertex,
        char const* vertex_entry_point,
        VkShaderModule fragment,
        char const* fragment_entry_point);

    VKHGraphicsPipelineBuilder& set_vertex_layout(VKHVertexLayout layout);

    VKHGraphicsPipelineBuilder& set_no_vertex_layout();

    VKHGraphicsPipelineBuilder& set_input_assembly(
        VkPrimitiveTopology topology,
        VkBool32 primitive_restart_enabled);

    VKHGraphicsPipelineBuilder& set_polygon_and_cull_mode(
        VkPolygonMode polygon_mode,
        VkCullModeFlags cull_mode,
        VkFrontFace front_face);

    VKHGraphicsPipelineBuilder& enable_depth_testing();

    VKHGraphicsPipelineBuilder& enable_color_blending();

    template<typename... Args>
    requires(std::is_same_v<Args, VkDynamicState> && ...)
    VKHGraphicsPipelineBuilder& set_dynamic_states(Args&&... args)
    {
        std::unordered_set<VkDynamicState> states(m_dynamic_states.begin(), m_dynamic_states.end());
        (states.insert(std::forward<Args>(args)), ...);

        m_dynamic_states.assign(states.begin(), states.end());

        m_dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        m_dynamic_state.dynamicStateCount = m_dynamic_states.size();
        m_dynamic_state.pDynamicStates = m_dynamic_states.data();

        return *this;
    }

    VkPipeline build();

private:
    void set_standard_viewport();

    void set_standard_rasterization();

    void set_standard_multisample();

    void set_standard_depth_testing();

    void set_standard_color_blending();

    void set_standard_pipeline_layout();

private:
    VkDevice m_device { nullptr };

    VkFormat m_color_format;
    VkFormat m_depth_format;
    VkFormat m_stencil_format;
    VkPipelineRenderingCreateInfo m_pipeline_rendering_info {};
    bool m_pipeline_rendering_info_done { false };

    VkPipelineShaderStageCreateInfo m_stages[VKH_SHADER_STAGE_COUNT] {};
    bool m_shader_stage_done { false };

    VkPipelineVertexInputStateCreateInfo m_vertex_input_state {};
    VKHVertexLayout m_vertex_layout;
    bool m_vertex_layout_done { false };

    VkPipelineInputAssemblyStateCreateInfo m_input_assembly_state {};
    bool m_input_assembly_done { false };

    VkPipelineViewportStateCreateInfo m_viewport_state {};

    VkPipelineRasterizationStateCreateInfo m_rasterization_state {};
    bool m_use_standard_rasterization { true };

    VkPipelineMultisampleStateCreateInfo m_multisample_state {};

    VkPipelineDepthStencilStateCreateInfo m_depthstencil_state {};
    bool m_use_depth { false };
    bool m_use_stencil { false };

    VkPipelineColorBlendAttachmentState m_color_attachment_state {};
    VkPipelineColorBlendStateCreateInfo m_color_blend_state {};
    bool m_use_color_blending { false };

    VkPipelineDynamicStateCreateInfo m_dynamic_state {};
    std::vector<VkDynamicState> m_dynamic_states;

    VkPipelineLayout m_pipeline_layout {};
};
