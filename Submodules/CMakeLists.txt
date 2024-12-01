# ThorVG

#project(Skia)
#
#add_library(Skia
#
#)
#
##target_sources(${PROJECT_NAME} PRIVATE
##)
#
#target_link_libraries(Skia
#    skia/out/mac-arm64/libskia.a
#)
#
#target_include_directories(Skia PUBLIC
#        skia/include
#)

project(ThorVG)

add_definitions(
        -DTHORVG_GL_TARGET_GLES
        -DTHORVG_LOG_ENABLED
)

add_library(${PROJECT_NAME}
        thorvg/src/common/tvgCompressor.cpp
        thorvg/src/common/tvgMath.cpp
        thorvg/src/common/tvgStr.cpp

#        Renderer
        thorvg/src/renderer/tvgAccessor.cpp
        thorvg/src/renderer/tvgAnimation.cpp
        thorvg/src/renderer/tvgCanvas.cpp
        thorvg/src/renderer/tvgFill.cpp
        thorvg/src/renderer/tvgGlCanvas.cpp
        thorvg/src/renderer/tvgInitializer.cpp
        thorvg/src/renderer/tvgLoader.cpp
        thorvg/src/renderer/tvgPaint.cpp
        thorvg/src/renderer/tvgPicture.cpp
        thorvg/src/renderer/tvgRender.cpp
        thorvg/src/renderer/tvgSaver.cpp
        thorvg/src/renderer/tvgScene.cpp
        thorvg/src/renderer/tvgShape.cpp
        thorvg/src/renderer/tvgSwCanvas.cpp
        thorvg/src/renderer/tvgTaskScheduler.cpp
        thorvg/src/renderer/tvgText.cpp

#       GL Renderer
        thorvg/src/renderer/gl_engine/tvgGlGeometry.cpp
        thorvg/src/renderer/gl_engine/tvgGlGpuBuffer.cpp
        thorvg/src/renderer/gl_engine/tvgGlProgram.cpp
        thorvg/src/renderer/gl_engine/tvgGlRenderer.cpp
        thorvg/src/renderer/gl_engine/tvgGlRenderPass.cpp
        thorvg/src/renderer/gl_engine/tvgGlRenderTarget.cpp
        thorvg/src/renderer/gl_engine/tvgGlRenderTask.cpp
        thorvg/src/renderer/gl_engine/tvgGlShader.cpp
        thorvg/src/renderer/gl_engine/tvgGlShaderSrc.cpp
        thorvg/src/renderer/gl_engine/tvgGlTessellator.cpp

#       SW Renderer
        thorvg/src/renderer/sw_engine/tvgSwFill.cpp
        thorvg/src/renderer/sw_engine/tvgSwImage.cpp
        thorvg/src/renderer/sw_engine/tvgSwMath.cpp
        thorvg/src/renderer/sw_engine/tvgSwMemPool.cpp
        thorvg/src/renderer/sw_engine/tvgSwPostEffect.cpp
        thorvg/src/renderer/sw_engine/tvgSwRaster.cpp
        thorvg/src/renderer/sw_engine/tvgSwRenderer.cpp
        thorvg/src/renderer/sw_engine/tvgSwRle.cpp
        thorvg/src/renderer/sw_engine/tvgSwShape.cpp
        thorvg/src/renderer/sw_engine/tvgSwStroke.cpp

#       Loaders
        thorvg/src/loaders/raw/tvgRawLoader.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
        thorvg/inc
)

target_include_directories(${PROJECT_NAME} PRIVATE
        thorvg_config
        thorvg/inc
        thorvg/src/common
        thorvg/src/renderer
        thorvg/src/renderer/gl_engine
        thorvg/src/renderer/sw_engine
        thorvg/src/loaders/raw
)


target_include_directories(${PROJECT_NAME} PUBLIC
        angle/mac/Headers
)

project(GLAD)

add_library(GLAD
        glad/src/glad.c
)

target_include_directories(GLAD PUBLIC
        glad/include
)
