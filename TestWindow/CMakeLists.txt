file(GLOB APP_SOURCES
    *.cpp
    *.h
)

# MFC dialog resources must be compiled into the EXE.
# `TestWindow.rc` includes `res/TestWindow.rc2` and references `res/TestWindow.ico`.
set(APP_RESOURCES
    TestWindow.rc
    res/TestWindow.rc2
    res/TestWindow.ico
)

add_executable(Example WIN32 ${APP_SOURCES} ${APP_RESOURCES})

target_link_libraries(Example PRIVATE svglib)

target_include_directories(Example PRIVATE
    ${CMAKE_SOURCE_DIR}/svglib
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/res
)

target_compile_definitions(Example PRIVATE _AFXDLL)
target_compile_definitions(Example PRIVATE UNICODE _UNICODE)

# Copy SVG assets after build
file(GLOB SVG_ASSETS assets/*.svg)

add_custom_command(TARGET Example POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:Example>/assets
    
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SVG_ASSETS}
        $<TARGET_FILE_DIR:Example>/assets
)