#include "vulkan_helper.h"

VKHVertexLayoutBuilder& VKHVertexLayoutBuilder::push_binding(
    uint32_t binding,
    uint32_t stride,
    VkVertexInputRate input_rate)
{
    VkVertexInputBindingDescription desc {};
    desc.binding = binding;
    desc.stride = stride;
    desc.inputRate = input_rate;

    m_binding_descs.push_back(desc);
    return *this;
}

VKHVertexLayoutBuilder& VKHVertexLayoutBuilder::push_attribute(
    uint32_t binding,
    uint32_t location,
    uint32_t offset,
    VkFormat format)
{
    VkVertexInputAttributeDescription desc {};
    desc.binding = binding;
    desc.location = location;
    desc.offset = offset;
    desc.format = format;

    m_attribute_descs.push_back(desc);
    return *this;
}

VKHVertexLayout VKHVertexLayoutBuilder::build()
{
    return { std::move(m_binding_descs), std::move(m_attribute_descs) };
}

VKHGraphicsPipelineBuilder::VKHGraphicsPipelineBuilder(VkDevice device)
    : m_device(device)
{
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::set_rendering_format(
    VkFormat color_format,
    VkFormat depth_format,
    VkFormat stencil_format)
{
    m_pipeline_rendering_info_done = true;

    m_color_format = color_format;
    m_depth_format = depth_format;
    m_stencil_format = stencil_format;

    m_pipeline_rendering_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    m_pipeline_rendering_info.colorAttachmentCount = 1;
    m_pipeline_rendering_info.pColorAttachmentFormats = &m_color_format;
    m_pipeline_rendering_info.depthAttachmentFormat = m_depth_format;
    m_pipeline_rendering_info.stencilAttachmentFormat = m_stencil_format;

    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::set_vertex_and_fragment(
    VkShaderModule vertex,
    char const* vertex_entry_point,
    VkShaderModule fragment,
    char const* fragment_entry_point)
{
    m_shader_stage_done = true;

    m_stages[VKH_SHADER_STAGE_VERTEX].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stages[VKH_SHADER_STAGE_VERTEX].stage = VK_SHADER_STAGE_VERTEX_BIT;
    m_stages[VKH_SHADER_STAGE_VERTEX].module = vertex;
    m_stages[VKH_SHADER_STAGE_VERTEX].pName = vertex_entry_point;
    m_stages[VKH_SHADER_STAGE_VERTEX].flags = 0;

    m_stages[VKH_SHADER_STAGE_FRAGMENT].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stages[VKH_SHADER_STAGE_FRAGMENT].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    m_stages[VKH_SHADER_STAGE_FRAGMENT].module = fragment;
    m_stages[VKH_SHADER_STAGE_FRAGMENT].pName = fragment_entry_point;
    m_stages[VKH_SHADER_STAGE_FRAGMENT].flags = 0;

    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::set_vertex_layout(VKHVertexLayout layout)
{
    m_vertex_layout_done = true;

    m_vertex_layout = std::move(layout);

    m_vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    m_vertex_input_state.vertexAttributeDescriptionCount = m_vertex_layout.attribute_descs.size();
    m_vertex_input_state.pVertexAttributeDescriptions = m_vertex_layout.attribute_descs.data();

    m_vertex_input_state.vertexBindingDescriptionCount = m_vertex_layout.binding_descs.size();
    m_vertex_input_state.pVertexBindingDescriptions = m_vertex_layout.binding_descs.data();

    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::set_no_vertex_layout()
{
    set_vertex_layout({});
    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::set_input_assembly(
    VkPrimitiveTopology topology,
    VkBool32 primitive_restart_enabled)
{
    m_input_assembly_done = true;

    m_input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_input_assembly_state.topology = topology;
    m_input_assembly_state.primitiveRestartEnable = primitive_restart_enabled;

    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::set_polygon_and_cull_mode(
    VkPolygonMode polygon_mode,
    VkCullModeFlags cull_mode,
    VkFrontFace front_face)
{
    m_use_standard_rasterization = false;

    m_rasterization_state.polygonMode = polygon_mode;
    m_rasterization_state.cullMode = cull_mode;
    m_rasterization_state.frontFace = front_face;

    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::enable_depth_testing()
{
    m_use_depth = true;
    return *this;
}

VKHGraphicsPipelineBuilder& VKHGraphicsPipelineBuilder::enable_color_blending()
{
    m_use_color_blending = true;
    return *this;
}

VkPipeline VKHGraphicsPipelineBuilder::build()
{
    if (!m_pipeline_rendering_info_done) {
        fmt::println(stderr, "VKHGraphicsPipelineBuilder::build(): rendering format is not specified!");
        return nullptr;
    }

    if (!m_shader_stage_done) {
        fmt::println(stderr, "VKHGraphicsPipelineBuilder::build(): shader stage is not specified!");
        return nullptr;
    }

    if (!m_vertex_layout_done) {
        fmt::println(stderr, "VKHGraphicsPipelineBuilder::build(): vertex layout is not specified!");
        return nullptr;
    }

    if (!m_input_assembly_done) {
        fmt::println(stderr, "VKHGraphicsPipelineBuilder::build(): input assembly is not specified!");
        return nullptr;
    }

    set_standard_viewport();
    set_standard_rasterization();
    set_standard_multisample();
    set_standard_depth_testing();
    set_standard_color_blending();
    set_standard_pipeline_layout();

    VkGraphicsPipelineCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.pNext = &m_pipeline_rendering_info;
    create_info.stageCount = VKH_SHADER_STAGE_COUNT;
    create_info.pStages = m_stages;
    create_info.pVertexInputState = &m_vertex_input_state;
    create_info.pInputAssemblyState = &m_input_assembly_state;
    create_info.pViewportState = &m_viewport_state;
    create_info.pRasterizationState = &m_rasterization_state;
    create_info.pMultisampleState = &m_multisample_state;
    create_info.pDepthStencilState = &m_depthstencil_state;
    create_info.pColorBlendState = &m_color_blend_state;
    create_info.pDynamicState = &m_dynamic_state;
    create_info.layout = m_pipeline_layout;

    VkPipeline pipeline;
    VK_CHECK(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));
    return pipeline;
}

void VKHGraphicsPipelineBuilder::set_standard_viewport()
{
    set_dynamic_states(VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR);

    m_viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_viewport_state.viewportCount = 1;
    m_viewport_state.scissorCount = 1;
}

void VKHGraphicsPipelineBuilder::set_standard_rasterization()
{
    if (m_use_standard_rasterization)
        set_polygon_and_cull_mode(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

    m_rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    m_rasterization_state.depthClampEnable = VK_FALSE;
    m_rasterization_state.depthBiasEnable = VK_FALSE;
    m_rasterization_state.depthBiasClamp = 0.0f;
    m_rasterization_state.depthBiasConstantFactor = 0.0f;
    m_rasterization_state.depthBiasSlopeFactor = 0.0f;
    m_rasterization_state.lineWidth = 1.0f;
}

void VKHGraphicsPipelineBuilder::set_standard_multisample()
{
    m_multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    m_multisample_state.sampleShadingEnable = VK_FALSE;
    m_multisample_state.minSampleShading = 1.0f;
    m_multisample_state.alphaToCoverageEnable = VK_FALSE;
    m_multisample_state.alphaToOneEnable = VK_FALSE;
}

void VKHGraphicsPipelineBuilder::set_standard_depth_testing()
{
    m_depthstencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    if (m_use_depth) {
        m_depthstencil_state.depthTestEnable = VK_TRUE;
        m_depthstencil_state.depthWriteEnable = VK_TRUE;
        m_depthstencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
        m_depthstencil_state.depthBoundsTestEnable = VK_FALSE;
    } else {
        m_depthstencil_state.depthTestEnable = VK_FALSE;
    }
}

void VKHGraphicsPipelineBuilder::set_standard_color_blending()
{
    if (m_use_color_blending) {
        m_color_attachment_state.blendEnable = VK_TRUE;
        m_color_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        m_color_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        m_color_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        m_color_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        m_color_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        m_color_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
    } else {
        m_color_attachment_state.blendEnable = VK_FALSE;
    }

    m_color_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT
        | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;

    m_color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_color_blend_state.logicOpEnable = VK_FALSE;
    m_color_blend_state.attachmentCount = 1;
    m_color_blend_state.pAttachments = &m_color_attachment_state;
}

void VKHGraphicsPipelineBuilder::set_standard_pipeline_layout()
{
    VkPipelineLayoutCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = 0;
    create_info.pushConstantRangeCount = 0;

    VK_CHECK(vkCreatePipelineLayout(m_device, &create_info, nullptr, &m_pipeline_layout));
}
