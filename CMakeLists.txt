cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(PathTracerGL VERSION 1.0)

set(HEADER_FILES
	"dependencies/imgui/imconfig.h"
	"dependencies/imgui/imgui.h"
	"dependencies/imgui/imgui_impl_glfw.h"
	"dependencies/imgui/imgui_impl_opengl3.h"
	"dependencies/imgui/imgui_impl_opengl3_loader.h"
	"dependencies/imgui/imgui_internal.h"
	"dependencies/imgui/imstb_rectpack.h"
	"dependencies/imgui/imstb_textedit.h"
	"dependencies/imgui/imstb_truetype.h"
	"src/application.h"
	"src/bvh.h"
	"src/camera.h"
	"src/framebuffer.h"
	"src/gui.h"
	"src/materials.h"
	"src/primitives.h"
	"src/renderer.h"
	"src/scene.h"
	"src/shader.h"
	"src/utils.h"
	"src/window.h"
)

set(SRC_FILES
	"dependencies/imgui/imgui.cpp"
	"dependencies/imgui/imgui_demo.cpp"
	"dependencies/imgui/imgui_draw.cpp"
	"dependencies/imgui/imgui_impl_glfw.cpp"
	"dependencies/imgui/imgui_impl_opengl3.cpp"
	"dependencies/imgui/imgui_tables.cpp"
	"dependencies/imgui/imgui_widgets.cpp"
	"src/main.cpp"
	"src/application.cpp"
	"src/bvh.cpp"
	"src/camera.cpp"
	"src/framebuffer.cpp"
	"src/gui.cpp"
	"src/primitives.cpp"
	"src/renderer.cpp"
	"src/scene.cpp"
	"src/shader.cpp"
	"src/utils.cpp"
	"src/window.cpp"
)

# Dependencies

# GLM
set(GLM_INCLUDE_PATH dependencies/glm)
include_directories(
	${GLM_INCLUDE_PATH}
)

# GLFW
add_subdirectory(dependencies/glfw EXCLUDE_FROM_ALL)
add_subdirectory(dependencies/)

# Add source to this project's executable.
add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES})

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} glad glfw ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES})

