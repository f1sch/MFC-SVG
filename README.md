# SvgLib

### Summary
This statically compiled C++ library can be integrated into Windows MFC applications and was created to explore graphical user interaction within dialog windows.

The goal was to simplify working with complex, deeply nested context menus: instead of reading through long menu structures, users can click directly on graphical elements to trigger actions faster and more intuitively.

SVG was chosen as the graphics format because SVG images are easy to design and customize. This makes it straightforward to adapt the visuals to specific application needs.

The library serves as an exploratory approach to graphics-driven interaction in classic MFC UIs, with a focus on improving usability for complex workflows.

## Installation
This project demonstrates how the library can be used in practice.

It includes example SVG files and a runnable sample application.

Build and run the project to get an impression of the libabry's functionality.

### Build and run
1. Clone the `MFC-SVG` repository
   
   ``` git clone https://github.com/f1sch/MFC-SVG ```

2. Setup build directory
   
   ``` mkdir build ```
   
   ``` cd build ```
3. CMake
    
    ``` cmake .. ```
    
    ``` cmake --build . --config Debug --target Example ```
4. Run
    
    ```.\MFC-SVG\build\TestWindow\Debug\Example.exe```

### Pictures

### License
MIT License

Copyright (c) 2026 Fritzn

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
