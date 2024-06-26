# 指定CMake的最低版本要求
project(Go1_Control)
cmake_minimum_required(VERSION 3.5)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bins/) #为CMakeLists.txt文件所在的目录。
# 检查系统架构
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "x86_64.*")
    # 如果系统架构是x86_64，设置ARCH变量为amd64
    set(ARCH amd64)
endif()
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "aarch64.*")
    # 如果系统架构是aarch64，设置ARCH变量为arm64
    set(ARCH arm64)
endif()

# 添加头文件目录，这样在编译时可以找到这些头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/unitree_legged_sdk)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/UnitreecameraSDK-main)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 添加链接库目录，这样在链接时可以找到这些库
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/unitree_legged_sdk/${ARCH})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/UnitreecameraSDK-main/${ARCH})
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
# 查找OpenCV库
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS}) # 从DIR改为DIRS

if(NOT OpenCV_FOUND)
    # 如果没有找到OpenCV，打印错误信息并退出
    message(FATAL_ERROR "OpenCV Library Not Found")
endif()

# 查找OpenGL库
find_package(OpenGL REQUIRED)
if(NOT OpenGL_FOUND)
    # 如果没有找到OpenGL，打印错误信息并退出
    message(FATAL_ERROR "OpenGL Library Not Found")
endif()

# 查找GLUT库
find_package(GLUT REQUIRED)
if(NOT GLUT_FOUND)
    # 如果没有找到GLUT，打印错误信息并退出
    message(FATAL_ERROR "GLUT Library Not Found")
endif()

# 查找X11库
find_package(X11 REQUIRED)
if(NOT X11_FOUND)
    # 如果没有找到X11，打印错误信息并退出
    message(FATAL_ERROR "X11 Library Not Found")
endif()

# 设置编译标志
set(EXTRA_LIBS -pthread libunitree_camera.a libunitree_legged_sdk.a tstc_V4L2_xu_camera udev systemlog ${OpenCV_LIBS} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${X11_LIBRARIES})
set(CMAKE_CXX_FLAGS "-O3 -fPIC")
set(CMAKE_CXX_STANDARD 14)

# 添加源文件，并生成可执行文件
add_executable(Go1_Control src/main.cpp include/DogVision.h include/DogMotion.h include/FrontCamera.h src/DogMotion.cpp src/DogVision.cpp src/FrontCamera.cpp)
# 链接库到可执行文件
target_link_libraries(Go1_Control ${EXTRA_LIBS})
# 安装指令，将可执行文件安装到指定目录
install(TARGETS Go1_Control DESTINATION bins)
