cd build
cmake --build . --config Debug
cmake --install . --config Debug --prefix ../../vcpkg/packages/zep_x64-windows-static-md

cmake --build . --config Release
cmake --install . --config Release --prefix ../../vcpkg/packages/zep_x64-windows-static-md

cmake --build . --config RelWithDebInfo
cmake --install . --config RelWithDebInfo --prefix ../../vcpkg/packages/zep_x64-windows-static-md
cd ..
