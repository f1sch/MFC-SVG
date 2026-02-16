# SvgLib

## Summary
This statically compiled C++ library can be integrated into Windows MFC applications and was created to explore graphical user interaction within dialog windows.

The goal was to simplify working with complex, deeply nested context menus: instead of reading through long menu structures, users can click directly on graphical elements to trigger actions faster and more intuitively.

SVG was chosen as the graphics format because SVG images are easy to design and customize. This makes it straightforward to adapt the visuals to specific application needs.

The library serves as an exploratory approach to graphics-driven interaction in classic MFC UIs, with a focus on improving usability for complex workflows.

## How to use
This project demonstrates how the library can be used in practice.

It includes example SVG files and a runnable sample application.

Build and run the project to get an impression of the libabry's functionality.

## How to build and run
Visual Studio: 
* Open the ProjectSVG.sln
* Build the Debug configuration
* Run

CMake:
* cd into root folder
* mkdir build
* cd build
* cmake ..
* cmake --build . --config Debug
* run MFC-SVG/build/TestWindow/Debug/Example.exe

## Pictures

## License

MIT
