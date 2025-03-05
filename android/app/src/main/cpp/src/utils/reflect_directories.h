#pragma once
#include <array>
#include <string_view>
#include <nvtx/nvtx3.hpp>

// This file is generated automatically, you should not modify it unless you know what you are doing.
constexpr std::array<std::pair<const char*, const char*>, 46> lookup_table{{
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\app.cpp","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\app.h","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\editor_p\\editor_layer.cpp","editor_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\editor_p\\editor_layer.h","editor_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\game_p\\game.cpp","game_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\game_p\\game.h","game_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\reflect_main.cpp","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\buffer\\vulkan_buffer.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\buffer\\vulkan_buffer.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\buffer\\vulkan_vertex_buffer.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\buffer\\vulkan_vertex_buffer.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\device\\debug_util.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\device\\debug_util.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\device\\vulkan_device.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\device\\vulkan_device.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\device\\vulkan_instance.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\device\\vulkan_instance.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\frame\\frame_data.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\frame\\frame_data.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\frame\\frame_resource_manager.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\frame\\frame_resource_manager.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\image\\image.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\image\\image.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\queues\\vulkan_queue.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\queues\\vulkan_queue.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\rasterizer_pipeline\\vertex.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\rasterizer_pipeline\\vertex.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\rasterizer_pipeline\\vulkan_rasterizer_pipeline.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\rasterizer_pipeline\\vulkan_rasterizer_pipeline.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\ray_tracing\\ray_tracer.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\ray_tracing\\ray_tracer.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\renderer.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\renderer.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\shader\\vulkan_shader.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\shader\\vulkan_shader.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\swap_chain\\vulkan_swap_chain.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\swap_chain\\vulkan_swap_chain.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\workers\\main_worker.cpp","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\renderer_p\\workers\\main_worker.h","renderer_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\scene_p\\scene_data.h","scene_p"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\utils\\log.h","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\utils\\other.h","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\utils\\pch.h","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\utils\\profiling_label.h","default"},
{"C:\\Users\\Natal\\Documents\\games\\reflect\\src\\utils\\ptr.h","default"},
}}; 
struct editor_p_domain { static constexpr char const* name{ "editor_p" }; };
struct game_p_domain { static constexpr char const* name{ "game_p" }; };
struct renderer_p_domain { static constexpr char const* name{ "renderer_p" }; };
struct scene_p_domain { static constexpr char const* name{ "scene_p" }; };
struct default_domain { static constexpr char const* name{ "default" }; };
#define GET_NVTX_DOMAIN(FILE) std::conditional_t<(std::string_view(find_file_category(FILE)) == "editor_p"),  editor_p_domain, std::conditional_t<(std::string_view(find_file_category(FILE)) == "game_p"),  game_p_domain, std::conditional_t<(std::string_view(find_file_category(FILE)) == "renderer_p"),  renderer_p_domain, std::conditional_t<(std::string_view(find_file_category(FILE)) == "scene_p"),  scene_p_domain, default_domain>>>>
constexpr const char * find_file_category(std::string_view key) {
    for (const auto& [k, v] : lookup_table) {
        if (k == key) return v;
    }
    return "default";
}
