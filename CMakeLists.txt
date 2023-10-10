cmake_minimum_required(VERSION 3.12)

project(JumaRE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(jutils)

option(JUMARE_USE_GLFW "Use GLFW lib for windows" ON)

option(JUMARE_BUILD_OPENGL "Build with OpenGL render API" ON)
option(JUMARE_BUILD_VULKAN "Build with Vulkan render API" OFF)
option(JUMARE_BUILD_DIRECTX11 "Build with DirectX11 render API" OFF)
option(JUMARE_BUILD_DIRECTX12 "Build with DirectX12 render API" OFF)

if((NOT UNIX) AND (JUMARE_BUILD_DIRECTX11 OR JUMARE_BUILD_DIRECTX12))
    set(JUMARE_BUILD_DIRECTX ON)
elseif()
    set(JUMARE_BUILD_DIRECTX OFF)
endif()

list(APPEND JUMARE_CORE_HEADER_FILES
    include/JumaRE/core.h
    include/JumaRE/render_target_id.h
    include/JumaRE/RenderAPI.h
    include/JumaRE/RenderEngine.h
    include/JumaRE/RenderEngineAsset.h
    include/JumaRE/RenderEngineContextObject.h
    include/JumaRE/RenderEngineImpl.h
    include/JumaRE/RenderEngineImpl_OpenGL.h
    include/JumaRE/RenderEngineImpl_Vulkan.h
    include/JumaRE/RenderEngineImpl_DirectX11.h
    include/JumaRE/RenderEngineImpl_DirectX12.h
    include/JumaRE/RenderOptions.h
    include/JumaRE/RenderPipeline.h
    include/JumaRE/RenderPrimitivesList.h
    include/JumaRE/RenderTarget.h

    include/JumaRE/input/InputButtons.h
    include/JumaRE/input/InputData.h

    include/JumaRE/material/Material.h
    include/JumaRE/material/MaterialParamsStorage.h
    include/JumaRE/material/MaterialProperties.h
    include/JumaRE/material/Shader.h
    include/JumaRE/material/ShaderCreateInfo.h
    include/JumaRE/material/ShaderUniform.h
    include/JumaRE/material/ShaderUniformInfo.h

    include/JumaRE/texture/Texture.h
    include/JumaRE/texture/TextureBase.h
    include/JumaRE/texture/TextureFormat.h
    include/JumaRE/texture/TextureSamplerType.h
    include/JumaRE/texture/TextureSamples.h

    include/JumaRE/vertex/VertexBuffer.h
    include/JumaRE/vertex/VertexBufferData.h
    include/JumaRE/vertex/VertexDescription.h

    include/JumaRE/window/window_id.h
    include/JumaRE/window/window_state_enums.h
    include/JumaRE/window/WindowController.h
)
list(APPEND JUMARE_GLFW_HEADER_FILES
    src/GLFW/WindowController_GLFW.h
)
list(APPEND JUMARE_OPENGL_HEADER_FILES
    src/OpenGL/Material_OpenGL.h
    src/OpenGL/RenderEngine_OpenGL.h
    src/OpenGL/RenderTarget_OpenGL.h
    src/OpenGL/Shader_OpenGL.h
    src/OpenGL/Texture_OpenGL.h
    src/OpenGL/TextureFormat_OpenGL.h
    src/OpenGL/VertexBuffer_OpenGL.h

    src/OpenGL/window/WindowController_OpenGL.h
    src/OpenGL/window/WindowController_OpenGL_GLFW.h
    src/OpenGL/window/WindowControllerImpl_OpenGL.h
)
list(APPEND JUMARE_VULKAN_HEADER_FILES
    src/Vulkan/Material_Vulkan.h
    src/Vulkan/RenderEngine_Vulkan.h
    src/Vulkan/RenderOptions_Vulkan.h
    src/Vulkan/RenderPipeline_Vulkan.h
    src/Vulkan/RenderTarget_Vulkan.h
    src/Vulkan/Shader_Vulkan.h
    src/Vulkan/Texture_Vulkan.h
    src/Vulkan/TextureFormat_Vulkan.h
    src/Vulkan/VertexBuffer_Vulkan.h

    src/Vulkan/vulkanObjects/VulkanBuffer.h
    src/Vulkan/vulkanObjects/VulkanCommandBuffer.h
    src/Vulkan/vulkanObjects/VulkanCommandPool.h
    src/Vulkan/vulkanObjects/VulkanFramebufferData.h
    src/Vulkan/vulkanObjects/VulkanImage.h
    src/Vulkan/vulkanObjects/VulkanQueueType.h
    src/Vulkan/vulkanObjects/VulkanRenderPass.h
    src/Vulkan/vulkanObjects/VulkanRenderPassDescription.h
    src/Vulkan/vulkanObjects/VulkanSwapchain.h

    src/Vulkan/window/WindowController_Vulkan.h
    src/Vulkan/window/WindowController_Vulkan_GLFW.h
    src/Vulkan/window/WindowControllerImpl_Vulkan.h
)
list(APPEND JUMARE_DIRECTX_HEADER_FILES
    src/DirectX/DirectXFunctions.h
    src/DirectX/TextureFormat_DirectX.h
)
list(APPEND JUMARE_DIRECTX11_HEADER_FILES
    src/DirectX11/Material_DirectX11.h
    src/DirectX11/RenderEngine_DirectX11.h
    src/DirectX11/RenderTarget_DirectX11.h
    src/DirectX11/Shader_DirectX11.h
    src/DirectX11/Texture_DirectX11.h
    src/DirectX11/VertexBuffer_DirectX11.h

    src/DirectX11/window/WindowController_DirectX11.h
    src/DirectX11/window/WindowController_DirectX11_GLFW.h
    src/DirectX11/window/WindowControllerImpl_DirectX11.h
)
list(APPEND JUMARE_DIRECTX12_HEADER_FILES
    src/DirectX12/D3D12MemAlloc.h
    src/DirectX12/Material_DirectX12.h
    src/DirectX12/RenderEngine_DirectX12.h
    src/DirectX12/RenderOptions_DirectX12.h
    src/DirectX12/RenderPipeline_DirectX12.h
    src/DirectX12/RenderTarget_DirectX12.h
    src/DirectX12/Shader_DirectX12.h
    src/DirectX12/Texture_DirectX12.h
    src/DirectX12/VertexBuffer_DirectX12.h

    src/DirectX12/DirectX12Objects/DirectX12Buffer.h
    src/DirectX12/DirectX12Objects/DirectX12CommandList.h
    src/DirectX12/DirectX12Objects/DirectX12CommandQueue.h
    src/DirectX12/DirectX12Objects/DirectX12MipGenerator.h
    src/DirectX12/DirectX12Objects/DirectX12MipGeneratorTarget.h
    src/DirectX12/DirectX12Objects/DirectX12PipelineStateStreamObjects.h
    src/DirectX12/DirectX12Objects/DirectX12Swapchain.h
    src/DirectX12/DirectX12Objects/DirectX12Texture.h

    src/DirectX12/window/WindowController_DirectX12.h
    src/DirectX12/window/WindowController_DirectX12_GLFW.h
    src/DirectX12/window/WindowControllerImpl_DirectX12.h
)

list(APPEND JUMARE_CORE_SOURCE_FILES
    src/core/InputData.cpp
    src/core/Material.cpp
    src/core/MaterialParamsStorage.cpp
    src/core/RenderEngine.cpp
    src/core/RenderEngineAsset.cpp
    src/core/RenderPipeline.cpp
    src/core/RenderTarget.cpp
    src/core/Shader.cpp
    src/core/Texture.cpp
    src/core/VertexBuffer.cpp
    src/core/WindowController.cpp
)
list(APPEND JUMARE_OPENGL_SOURCE_FILES
    src/OpenGL/Material_OpenGL.cpp
    src/OpenGL/RenderEngine_OpenGL.cpp
    src/OpenGL/RenderEngineImpl_OpenGL.cpp
    src/OpenGL/RenderTarget_OpenGL.cpp
    src/OpenGL/Shader_OpenGL.cpp
    src/OpenGL/Texture_OpenGL.cpp
    src/OpenGL/VertexBuffer_OpenGL.cpp

    src/OpenGL/window/WindowController_OpenGL.cpp
    src/OpenGL/window/WindowController_OpenGL_GLFW.cpp
    src/OpenGL/window/WindowControllerImpl_OpenGL.cpp
)
list(APPEND JUMARE_VULKAN_SOURCE_FILES
    src/Vulkan/Material_Vulkan.cpp
    src/Vulkan/RenderEngine_Vulkan.cpp
    src/Vulkan/RenderEngineImpl_Vulkan.cpp
    src/Vulkan/RenderPipeline_Vulkan.cpp
    src/Vulkan/RenderTarget_Vulkan.cpp
    src/Vulkan/Shader_Vulkan.cpp
    src/Vulkan/Texture_Vulkan.cpp
    src/Vulkan/VertexBuffer_Vulkan.cpp

    src/Vulkan/vulkanObjects/VulkanBuffer.cpp
    src/Vulkan/vulkanObjects/VulkanCommandBuffer.cpp
    src/Vulkan/vulkanObjects/VulkanCommandPool.cpp
    src/Vulkan/vulkanObjects/VulkanImage.cpp
    src/Vulkan/vulkanObjects/VulkanRenderPass.cpp
    src/Vulkan/vulkanObjects/VulkanSwapchain.cpp

    src/Vulkan/window/WindowController_Vulkan.cpp
    src/Vulkan/window/WindowController_Vulkan_GLFW.cpp
    src/Vulkan/window/WindowControllerImpl_Vulkan.cpp
)
list(APPEND JUMARE_DIRECTX_SOURCE_FILES
    src/DirectX/DirectXFunctions.cpp
)
list(APPEND JUMARE_DIRECTX11_SOURCE_FILES
    src/DirectX11/Material_DirectX11.cpp
    src/DirectX11/RenderEngine_DirectX11.cpp
    src/DirectX11/RenderEngineImpl_DirectX11.cpp
    src/DirectX11/RenderTarget_DirectX11.cpp
    src/DirectX11/Shader_DirectX11.cpp
    src/DirectX11/Texture_DirectX11.cpp
    src/DirectX11/VertexBuffer_DirectX11.cpp

    src/DirectX11/window/WindowController_DirectX11.cpp
    src/DirectX11/window/WindowController_DirectX11_GLFW.cpp
    src/DirectX11/window/WindowControllerImpl_DirectX11.cpp
)
list(APPEND JUMARE_DIRECTX12_SOURCE_FILES
    src/DirectX12/D3D12MemAlloc.cpp
    src/DirectX12/Material_DirectX12.cpp
    src/DirectX12/RenderEngine_DirectX12.cpp
    src/DirectX12/RenderEngineImpl_DirectX12.cpp
    src/DirectX12/RenderPipeline_DirectX12.cpp
    src/DirectX12/RenderTarget_DirectX12.cpp
    src/DirectX12/Shader_DirectX12.cpp
    src/DirectX12/Texture_DirectX12.cpp
    src/DirectX12/VertexBuffer_DirectX12.cpp

    src/DirectX12/DirectX12Objects/DirectX12Buffer.cpp
    src/DirectX12/DirectX12Objects/DirectX12CommandList.cpp
    src/DirectX12/DirectX12Objects/DirectX12CommandQueue.cpp
    src/DirectX12/DirectX12Objects/DirectX12MipGenerator.cpp
    src/DirectX12/DirectX12Objects/DirectX12MipGeneratorTarget.cpp
    src/DirectX12/DirectX12Objects/DirectX12Swapchain.cpp
    src/DirectX12/DirectX12Objects/DirectX12Texture.cpp

    src/DirectX12/window/WindowController_DirectX12.cpp
    src/DirectX12/window/WindowController_DirectX12_GLFW.cpp
    src/DirectX12/window/WindowControllerImpl_DirectX12.cpp
)

list(APPEND JUMARE_SOURCE_FILES ${JUMARE_CORE_SOURCE_FILES})
if(JUMARE_USE_GLFW)
    find_package(glfw3 REQUIRED)
    list(APPEND JUMARE_PRIVATE_HEADER_FILES ${JUMARE_GLFW_HEADER_FILES})
    list(APPEND JUMARE_MACRO_DEFINITIONS GLFW_ENABLED)
    list(APPEND JUMARE_LIBS glfw)
endif()
if(JUMARE_BUILD_OPENGL)
    find_package(GLEW REQUIRED)
    list(APPEND JUMARE_PRIVATE_HEADER_FILES ${JUMARE_OPENGL_HEADER_FILES})
    list(APPEND JUMARE_SOURCE_FILES ${JUMARE_OPENGL_SOURCE_FILES})
    list(APPEND JUMARE_MACRO_DEFINITIONS JUMARE_ENABLE_OPENGL)
    if(UNIX)
        list(APPEND JUMARE_LIBS gl glew)
    else()
        list(APPEND JUMARE_LIBS glew32s)
    endif()
endif()
if(JUMARE_BUILD_VULKAN)
    find_package(Vulkan REQUIRED)
    list(APPEND JUMARE_PRIVATE_HEADER_FILES ${JUMARE_VULKAN_HEADER_FILES})
    list(APPEND JUMARE_SOURCE_FILES ${JUMARE_VULKAN_SOURCE_FILES})
    list(APPEND JUMARE_MACRO_DEFINITIONS JUMARE_ENABLE_VULKAN)
    if(UNIX)
        list(APPEND JUMARE_LIBS vulkan)
    else()
        list(APPEND JUMARE_LIBS vulkan-1)
    endif()
endif()
if(JUMARE_BUILD_DIRECTX)
    list(APPEND JUMARE_PRIVATE_HEADER_FILES ${JUMARE_DIRECTX_HEADER_FILES})
    list(APPEND JUMARE_SOURCE_FILES ${JUMARE_DIRECTX_SOURCE_FILES})
    list(APPEND JUMARE_MACRO_DEFINITIONS JUMARE_ENABLE_DX)
    if(JUMARE_BUILD_DIRECTX11)
        list(APPEND JUMARE_PRIVATE_HEADER_FILES ${JUMARE_DIRECTX11_HEADER_FILES})
        list(APPEND JUMARE_SOURCE_FILES ${JUMARE_DIRECTX11_SOURCE_FILES})
        list(APPEND JUMARE_MACRO_DEFINITIONS JUMARE_ENABLE_DX11)
    endif()
    if(JUMARE_BUILD_DIRECTX12)
        list(APPEND JUMARE_PRIVATE_HEADER_FILES ${JUMARE_DIRECTX12_HEADER_FILES})
        list(APPEND JUMARE_SOURCE_FILES ${JUMARE_DIRECTX12_SOURCE_FILES})
        list(APPEND JUMARE_MACRO_DEFINITIONS JUMARE_ENABLE_DX12)
    endif()
endif()

add_library(JumaRE STATIC ${JUMARE_SOURCE_FILES})
target_compile_definitions(JumaRE PRIVATE ${JUMARE_MACRO_DEFINITIONS})
target_include_directories(JumaRE PUBLIC include)
target_link_libraries(JumaRE PUBLIC jutils PRIVATE ${JUMARE_LIBS})