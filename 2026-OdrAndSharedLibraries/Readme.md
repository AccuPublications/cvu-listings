# OdrAndSharedLibraries readme file

These files accompany the article to the published in the ACCU magazines.

# Build instructions, using CMake

The CMakeLists.txt file in this directory is standard and those who are familiar with CMake are unlikely to need any further guidance.

For those who do:

1. Prepare for the build using `cmake -S . -B build` on Windows (using MSVC) or Linux (using gcc)
Other enviroments may also work, but have not been tested.

1. Run the build using `cmake --build build`

1. Install the built files in the `bin` subdirectory using:
`cmake --install build --prefix .` (Linux)
or 
`cmake --install build --prefix . --config Debug` (Windows)
