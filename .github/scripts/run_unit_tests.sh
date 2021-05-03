#!/usr/bin/env bash

echo "Starting unit tests"
copy SDL2/lib/x64/SDL2.dll x64/Release # Copy SDL2 DLL before running
./x64/Release/Chonkystation.exe "tests/test_r0/test_r0.exe" --continuous-integration # Run first test

if [ $? -ne 0 ]; then
    echo "$zero test: Failed"
    exit -1
else
	echo "$zero test: Passed!"
fi


echo "Done testing!"