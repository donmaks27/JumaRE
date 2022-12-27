cmake_minimum_required(VERSION 3.12)

project(JumaRE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(JUMARE_USE_GLFW "Use GLFW lib for windows" ON)

option(JUMARE_BIULD_OPENGL "Build with OpenGL render API" ON)
option(JUMARE_BUILD_VULKAN "Build with Vulkan render API" OFF)

list(APPEND JUMARE_CPP_FILES
    src/core/InputData.cpp
    src/core/Shader.cpp
    src/core/Material.cpp
    src/core/MaterialParamsStorage.cpp
    src/core/VertexBuffer.cpp
    src/core/Texture.cpp
    src/core/RenderTarget.cpp
    src/core/RenderPipeline.cpp
    src/core/WindowController.cpp
    src/core/RenderEngine.cpp
    src/OpenGL/RenderEngineImpl_OpenGL.cpp
    src/Vulkan/RenderEngineImpl_Vulkan.cpp
    src/DirectX11/RenderEngineImpl_DirectX11.cpp
    src/DirectX12/RenderEngineImpl_DirectX12.cpp
)
list(APPEND JUMARE_LIBS jutils)

if(JUMARE_USE_GLFW)
    list(APPEND JUMARE_MACRO_DEFINITIONS GLFW_ENABLED)
    list(APPEND JUMARE_LIBS glfw)
endif()

if(JUMARE_BIULD_OPENGL)
    list(APPEND JUMARE_CPP_FILES
        src/OpenGL/Shader_OpenGL.cpp 
        src/OpenGL/Material_OpenGL.cpp  
        src/OpenGL/VertexBuffer_OpenGL.cpp 
        src/OpenGL/Texture_OpenGL.cpp
        src/OpenGL/RenderTarget_OpenGL.cpp
        src/OpenGL/RenderEngine_OpenGL.cpp 
        src/OpenGL/window/WindowController_OpenGL.cpp
        src/OpenGL/window/WindowController_OpenGL_GLFW.cpp
        src/OpenGL/window/WindowControllerImpl_OpenGL.cpp
    )
    list(APPEND JUMARE_MACRO_DEFINITIONS JUMARE_ENABLE_OPENGL)
    list(APPEND JUMARE_LIBS GL GLEW)
endif()

add_library(JumaRE STATIC ${JUMARE_CPP_FILES})
target_compile_definitions(JumaRE PRIVATE ${JUMARE_MACRO_DEFINITIONS})
target_include_directories(JumaRE INTERFACE $<TARGET_PROPERTY:jutils,INTERFACE_INCLUDE_DIRECTORIES> include)
target_link_libraries(JumaRE PRIVATE ${JUMARE_LIBS})

install(DIRECTORY include/JumaRE/ DESTINATION include/JumaRE FILES_MATCHING PATTERN "*.h")
install(TARGETS JumaRE DESTINATION lib)