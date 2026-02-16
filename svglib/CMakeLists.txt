file(GLOB SVGLIB_SOURCES
    *.cpp
    *.h
)

add_library(svglib STATIC ${SVGLIB_SOURCES})

target_include_directories(svglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Required for MFC + static lib
target_compile_definitions(svglib PRIVATE _AFXDLL)

target_compile_definitions(svglib PRIVATE UNICODE _UNICODE)