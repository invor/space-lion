cmake_minimum_required(VERSION 3.16)

project(space-lion)

set(CMAKE_CXX_STANDARD 23)

option(USE_GL "Use OpenGL graphics backend" ON)
option(USE_DX11 "Use DirectX11 graphics backend" OFF)
# Add compile defintion for UWP via cmake optioon
option(UWP "Compile for UWP" OFF)

if(UWP)
add_compile_definitions(_UWP)
#add_compile_definitions(__cplusplus_winrt)
endif()

if(USE_GL)

# try adding glfw by its own cmake file
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(src/External/glfw)

# try adding glowl by its own cmake file
set(GLOWL_OPENGL_INCLUDE "GLAD" CACHE STRING "Choose OpenGL include.")
add_subdirectory(src/External/glowl EXCLUDE_FROM_ALL)

endif()

if(USE_DX11)

# try adding dxowl by its own cmake file
add_subdirectory(src/External/dxowl EXCLUDE_FROM_ALL)

endif()


SET (ENGINECORE_ANIMATION_HEADER_FILES
        src/EngineCore/TurntableComponentManager.hpp
        src/EngineCore/AnimationSystems.hpp
        src/EngineCore/SkinComponentManager.hpp)

SET (ENGINECORE_ANIMATION_SOURCE_FILES
        src/EngineCore/TurntableComponentManager.cpp
        src/EngineCore/AnimationSystems.cpp
        src/EngineCore/SkinComponentManager.cpp)

SET (ENGINECORE_COMMON_HEADER_FILES
        src/EngineCore/BaseComponentManager.hpp
        src/EngineCore/BaseMultiInstanceComponentManager.hpp
        src/EngineCore/BaseSingleInstanceComponentManager.hpp
        src/EngineCore/BSplineComponent.hpp
        src/EngineCore/EntityManager.hpp
        src/EngineCore/Frame.hpp
        src/EngineCore/InputEvent.hpp
        src/EngineCore/NameComponentManager.hpp
        src/EngineCore/TransformComponentManager.hpp
        src/EngineCore/WorldState.hpp)

SET (ENGINECORE_COMMON_SOURCE_FILES
        src/EngineCore/BSplineComponent.cpp
        src/EngineCore/EntityManager.cpp
        src/EngineCore/Frame.cpp
        src/EngineCore/NameComponentManager.cpp
        src/EngineCore/TransformComponentManager.cpp
        src/EngineCore/WorldState.cpp)

SET (ENGINECORE_GRAPHICS_HEADER_FILES
        src/EngineCore/AtmosphereComponentManager.hpp
        src/EngineCore/BaseResourceManager.hpp
        src/EngineCore/CameraComponent.hpp
        src/EngineCore/GenericTextureLayout.hpp
        src/EngineCore/GenericVertexLayout.hpp
        src/EngineCore/GeometryBakery.hpp
        src/EngineCore/gltfAssetComponentManager.hpp
        src/EngineCore/MaterialComponentManager.hpp
        src/EngineCore/MeshComponentManager.hpp
        src/EngineCore/OceanComponent.hpp
        src/EngineCore/PointlightComponent.hpp
        src/EngineCore/RenderPass.hpp
        src/EngineCore/RenderTaskComponentManager.hpp
        src/EngineCore/SunlightComponentManager.hpp
        src/EngineCore/LandscapeFeatureCurveComponent.hpp
        #src/EngineCore/LandscapeBrickComponent.hpp
)

SET (ENGINECORE_GRAPHICS_SOURCE_FILES
        src/EngineCore/AtmosphereComponentManager.cpp
        src/EngineCore/CameraComponent.cpp
        src/EngineCore/GeometryBakery.cpp
        src/EngineCore/gltfAssetComponentManager.cpp
        src/EngineCore/MaterialComponentManager.cpp
        src/EngineCore/MeshComponentManager.cpp
        src/EngineCore/OceanComponent.cpp
        src/EngineCore/PointlightComponent.cpp
        src/EngineCore/RenderPass.cpp
        src/EngineCore/RenderTaskComponentManager.cpp
        src/EngineCore/SunlightComponentManager.cpp
        src/EngineCore/LandscapeFeatureCurveComponent.inl
        #src/EngineCore/LandscapeBrickComponent.inl
)

SET (ENGINECORE_GRAPHICS_GL_HEADER_FILES
        src/EngineCore/OpenGL/AtmosphereRenderPass.hpp
        src/EngineCore/OpenGL/BasicRenderingPipeline.hpp
        src/EngineCore/OpenGL/GraphicsBackend.hpp
	#src/EngineCore/OpenGL/LandscapeSystems.hpp
        src/EngineCore/OpenGL/OceanRenderPass.hpp
        src/EngineCore/OpenGL/ResourceManager.hpp
        src/EngineCore/OpenGL/SkinnedMeshRenderPass.hpp)

SET (ENGINECORE_GRAPHICS_GL_SOURCE_FILES
        src/EngineCore/OpenGL/AtmosphereRenderPass.cpp
        src/EngineCore/OpenGL/BasicRenderingPipeline.cpp
        src/EngineCore/OpenGL/GraphicsBackend.cpp
	#src/EngineCore/OpenGL/LandscapeSystems.cpp
        src/EngineCore/OpenGL/OceanRenderPass.cpp
        src/EngineCore/OpenGL/ResourceManager.cpp
        src/EngineCore/OpenGL/SkinnedMeshRenderPass.cpp)

SET (ENGINECORE_GRAPHICS_DX11_HEADER_FILES
        src/EngineCore/Dx11/Dx11Frame.hpp
        src/EngineCore/Dx11/SimpleForwardRenderingPipeline.hpp
        src/EngineCore/Dx11/ResourceManager.hpp)

SET (ENGINECORE_GRAPHICS_DX11_SOURCE_FILES
        src/EngineCore/Dx11/SimpleForwardRenderingPipeline.cpp
        src/EngineCore/Dx11/ResourceManager.cpp)

SET (ENGINECORE_PHYSICS_HEADER_FILES
        src/EngineCore/AirplanePhysicsComponent.hpp)

SET (ENGINECORE_PHYSICS_SOURCE_FILES
        src/EngineCore/AirplanePhysicsComponent.cpp)

SET (ENGINECORE_UTILITY_HEADER_FILES
        src/EngineCore/MTQueue.hpp
        src/EngineCore/ResourceLoading.hpp
        src/EngineCore/TaskSchedueler.hpp
        src/EngineCore/types.hpp
        src/EngineCore/utility.hpp)

SET (ENGINECORE_UTILITY_SOURCE_FILES
        src/EngineCore/ResourceLoading.cpp
        src/EngineCore/TaskSchedueler.cpp)

SET (EDITOR_FILES
        src/Editor/CameraController.hpp
        src/Editor/CameraController.cpp)

SET (DX11_VERTEX_SHADER_FILES
   resources/shaders/hlsl/static_mesh.vertex.hlsl
   resources/shaders/hlsl/static_mesh_unlit.vertex.hlsl)

SET (DX11_PIXEL_SHADER_FILES
   resources/shaders/hlsl/forward_pbr.pixel.hlsl
   resources/shaders/hlsl/textured_unlit.pixel.hlsl)

SET (DX11_SHADER_FILES ${DX11_VERTEX_SHADER_FILES} ${DX11_PIXEL_SHADER_FILES})

add_library(SpaceLion "")

add_definitions(-DNOMINMAX)

target_include_directories(
        SpaceLion PUBLIC
        "${PROJECT_SOURCE_DIR}/src/Editor"
        "${PROJECT_SOURCE_DIR}/src/EngineCore"
        "${PROJECT_SOURCE_DIR}/src/External/tinygltf"
        "${PROJECT_SOURCE_DIR}/src/External/glm"
        )

target_sources(SpaceLion
    PRIVATE
        ${EDITOR_FILES}
        ${ENGINECORE_ANIMATION_HEADER_FILES}
        ${ENGINECORE_ANIMATION_SOURCE_FILES}
        ${ENGINECORE_COMMON_HEADER_FILES}
        ${ENGINECORE_COMMON_SOURCE_FILES}
        ${ENGINECORE_GRAPHICS_HEADER_FILES}
        ${ENGINECORE_GRAPHICS_SOURCE_FILES}
        ${ENGINECORE_PHYSICS_HEADER_FILES}
        ${ENGINECORE_PHYSICS_SOURCE_FILES}
        ${ENGINECORE_UTILITY_HEADER_FILES}
        ${ENGINECORE_UTILITY_SOURCE_FILES}
)

source_group(Editor FILES ${EDITOR_FILES})
source_group(EngineCore\\Animation FILES ${ENGINECORE_ANIMATION_SOURCE_FILES} ${ENGINECORE_ANIMATION_HEADER_FILES})
source_group(EngineCore\\Common FILES ${ENGINECORE_COMMON_HEADER_FILES} ${ENGINECORE_COMMON_SOURCE_FILES})
source_group(EngineCore\\Graphics FILES ${ENGINECORE_GRAPHICS_HEADER_FILES} ${ENGINECORE_GRAPHICS_SOURCE_FILES})
source_group(EngineCore\\Physics FILES ${ENGINECORE_PHYSICS_HEADER_FILES} ${ENGINECORE_PHYSICS_SOURCE_FILES})
source_group(EngineCore\\Utility FILES ${ENGINECORE_UTILITY_HEADER_FILES} ${ENGINECORE_UTILITY_SOURCE_FILES})

SET (EXTERNAL_LODEPNG_FILES
        src/External/lodepng/lodepng.h
        src/External/lodepng/lodepng.cpp
)

target_include_directories(
        SpaceLion PUBLIC
        "${PROJECT_SOURCE_DIR}/src/External/lodepng/"
)

source_group(External\\lodepng\\ FILES ${EXTERNAL_LODEPNG_FILES})

SET (EXTERNAL_IMPLOT_FILES
        src/External/implot/implot.h
        src/External/implot/implot.cpp
        src/External/implot/implot_demo.cpp
        src/External/implot/implot_internal.h
        src/External/implot/implot_items.cpp
)

target_include_directories(
        SpaceLion PUBLIC
        "${PROJECT_SOURCE_DIR}/src/External/implot/"
)

target_sources(SpaceLion
    PRIVATE
        ${EXTERNAL_IMPLOT_FILES}
)

source_group(External\\implot\\ FILES ${EXTERNAL_IMPLOT_FILES})

if(USE_GL)

SET (EXTERNAL_GLAD_FILES
        src/External/glad/include/glad/glad.h
        src/External/glad/include/KHR/khrplatform.h
        src/External/glad/src/glad.c
)

SET (EXTERNAL_IMGUI_FILES
        src/External/imgui/imconfig.h
        src/External/imgui/imgui.h
        src/External/imgui/imgui.cpp
        src/External/imgui/imgui_demo.cpp
        src/External/imgui/imgui_draw.cpp
        src/External/imgui/imgui_widgets.cpp
        src/External/imgui/backends/imgui_impl_glfw.h
        src/External/imgui/backends/imgui_impl_glfw.cpp
        src/External/imgui/backends/imgui_impl_opengl3.h
        src/External/imgui/backends/imgui_impl_opengl3.cpp
        src/External/imgui/imstb_rectpack.h
        src/External/imgui/imstb_textedit.h
        src/External/imgui/imstb_truetype.h
)

SET (EXTERNAL_GLOWL_FILES
        src/External/glowl/include/glowl/BufferObject.hpp
        src/External/glowl/include/glowl/Exceptions.hpp
        src/External/glowl/include/glowl/FramebufferObject.hpp
        src/External/glowl/include/glowl/glowl.h
        src/External/glowl/include/glowl/GLSLProgram.hpp
        src/External/glowl/include/glowl/Mesh.hpp
        src/External/glowl/include/glowl/Texture.hpp
        src/External/glowl/include/glowl/Texture2D.hpp
        src/External/glowl/include/glowl/Texture2DView.hpp
        src/External/glowl/include/glowl/Texture2DArray.hpp
        src/External/glowl/include/glowl/Texture3D.hpp
        src/External/glowl/include/glowl/Texture3DView.hpp
        src/External/glowl/include/glowl/TextureCubemapArray.hpp
        src/External/glowl/include/glowl/VertexLayout.hpp
)

target_include_directories(
        SpaceLion PUBLIC
        "${PROJECT_SOURCE_DIR}/src/External/glad/include"
        "${PROJECT_SOURCE_DIR}/src/External/glfw/include"
        "${PROJECT_SOURCE_DIR}/src/External/imgui/"
        "${PROJECT_SOURCE_DIR}/src/External/lodepng/"
)

target_link_libraries(SpaceLion PUBLIC glowl glfw ${GLFW_LIBRARIES})

target_sources(SpaceLion
    PRIVATE
        ${ENGINECORE_GRAPHICS_GL_HEADER_FILES}
        ${ENGINECORE_GRAPHICS_GL_SOURCE_FILES}
        ${EXTERNAL_GLAD_FILES}
        ${EXTERNAL_IMGUI_FILES}
        ${EXTERNAL_LODEPNG_FILES}
        ${EXTERNAL_GLOWL_FILES}
)

source_group(EngineCore\\Graphics\\OpenGL FILES ${ENGINECORE_GRAPHICS_GL_HEADER_FILES} ${ENGINECORE_GRAPHICS_GL_SOURCE_FILES})
source_group(External\\glad\\ FILES ${EXTERNAL_GLAD_FILES})
source_group(External\\imgui\\ FILES ${EXTERNAL_IMGUI_FILES})
source_group(External\\glowl\\ FILES ${EXTERNAL_GLOWL_FILES})

endif()

if(USE_DX11)

set_property(SOURCE ${DX11_SHADER_FILES} PROPERTY VS_SHADER_ENTRYPOINT Main)
set_property(SOURCE ${DX11_SHADER_FILES} PROPERTY VS_SHADER_MODEL 5.0)
set_property(SOURCE ${DX11_SHADER_FILES} PROPERTY VS_SHADER_FLAGS /Zi)

set_property(SOURCE ${DX11_VERTEX_SHADER_FILES} PROPERTY VS_SHADER_TYPE Vertex)
set_property(SOURCE ${DX11_PIXEL_SHADER_FILES} PROPERTY VS_SHADER_TYPE Pixel)

SET (EXTERNAL_IMGUI_FILES
        src/External/imgui/imconfig.h
        src/External/imgui/imgui.h
        src/External/imgui/imgui.cpp
        src/External/imgui/imgui_demo.cpp
        src/External/imgui/imgui_draw.cpp
        src/External/imgui/imgui_internal.h
        src/External/imgui/imgui_tables.cpp
        src/External/imgui/imgui_widgets.cpp
        src/External/imgui/backends/imgui_impl_dx11.h
        src/External/imgui/backends/imgui_impl_dx11.cpp
        src/External/imgui/imstb_rectpack.h
        src/External/imgui/imstb_textedit.h
        src/External/imgui/imstb_truetype.h
)

SET (EXTERNAL_DXOWL_FILES
        src/External/dxowl/include/dxowl/Buffer.hpp
        src/External/dxowl/include/dxowl/RenderTarget.hpp
        src/External/dxowl/include/dxowl/ShaderProgram.hpp
        src/External/dxowl/include/dxowl/Mesh.hpp
        src/External/dxowl/include/dxowl/Texture2D.hpp
        src/External/dxowl/include/dxowl/Texture3D.hpp
        src/External/dxowl/include/dxowl/VertexDescriptor.hpp
)

target_include_directories(
        SpaceLion PUBLIC
        "${PROJECT_SOURCE_DIR}/src/External/imgui/"
)

target_link_libraries(SpaceLion PUBLIC dxowl)

target_sources(SpaceLion
    PRIVATE
        ${ENGINECORE_GRAPHICS_DX11_HEADER_FILES}
        ${ENGINECORE_GRAPHICS_DX11_SOURCE_FILES}
        ${EXTERNAL_IMGUI_FILES}
        ${EXTERNAL_DXOWL_FILES}
        ${DX11_SHADER_FILES}
)

source_group(EngineCore\\Graphics\\DX11 FILES ${ENGINECORE_GRAPHICS_DX11_HEADER_FILES} ${ENGINECORE_GRAPHICS_DX11_SOURCE_FILES})
source_group(External\\imgui\\ FILES ${EXTERNAL_IMGUI_FILES})
source_group(External\\dxowl\\ FILES ${EXTERNAL_DXOWL_FILES})
source_group(Resources\\Shaders\\ FILES ${DX11_SHADER_FILES})

endif()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)



# for now, in place example
#add_executable(example "")
#
#target_sources(example
#    PRIVATE
#        src/EngineCore/main.cpp
#)
#
#target_link_libraries(example PUBLIC EngineCore)
#
#target_include_directories(example PUBLIC
#                          "${PROJECT_BINARY_DIR}"
#                          "${PROJECT_SOURCE_DIR}/src/EngineCore"
#                          )