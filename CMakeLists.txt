cmake_minimum_required(VERSION 3.10)
project(lvgl)

foreach(BACKEND_NAME "SDL" "LINUX_DRM" "LINUX_FBDEV" "X11" "WAYLAND")
    execute_process(COMMAND "scripts/backend_conf.sh" ${BACKEND_NAME} OUTPUT_VARIABLE IS_BACKEND_ENABLED)
    set("LV_USE_${BACKEND_NAME}" ${IS_BACKEND_ENABLED})
endforeach()

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

# Uncomment if the program needs debugging
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")

set(CMAKE_C_STANDARD 99) # LVGL officially supports C99 and above
set(CMAKE_CXX_STANDARD 17) #C17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${PROJECT_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)

find_package(OpenCV REQUIRED)

# 设置PKG查找路径
# set(ENV{PKG_CONFIG_PATH} "/usr/lib/pkgconfig")

pkg_check_modules(LIBAVCODEC REQUIRED libavcodec)
pkg_check_modules(LIBAVFORMAT REQUIRED libavformat)
pkg_check_modules(LIBAVFILTER REQUIRED libavfilter)
pkg_check_modules(LIBAVUTIL REQUIRED libavutil)
pkg_check_modules(LIBSWSCALE REQUIRED libswscale)
# freetype
# pkg_check_modules(LIBFREETYPE REQUIRED freetype2)

file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)

file(GLOB_RECURSE IMAGE_FILES ${SOURCE_DIR}/assets/images/*.c)

target_include_directories(lvgl PUBLIC ${LIBDRM_INCLUDE_DIRS} ${SOURCE_DIR}/include ${SOURCE_DIR}/library)
add_executable(lvglsim ${SOURCE_FILES} ${IMAGE_FILES})
target_link_libraries(lvglsim
    lvgl
    lvgl::examples
    lvgl::demos 
    lvgl::thorvg 
    ${LIBDRM_LIBRARIES} 
    m 
    pthread 
    ${OpenCV_LIBS} 
    ${LIBAVCODEC_LIBRARIES} 
    ${LIBAVFORMAT_LIBRARIES} 
    ${LIBAVFILTER_LIBRARIES} 
    ${LIBAVUTIL_LIBRARIES} 
    ${LIBSWSCALE_LIBRARIES} 
    # ${LIBFREETYPE_LIBRARIES}
)

add_custom_target (run COMMAND ${EXECUTABLE_OUTPUT_PATH}/lvglsim DEPENDS lvglsim)
