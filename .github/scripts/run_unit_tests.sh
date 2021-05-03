#!/usr/bin/env bash

echo "Starting unit tests"
ls # List directory tree
./x64/Release/Chonkystation.exe "tests/test_r0/test_r0.exe" --continuous-integration # Run first test

if [ $? -ne 0 ]; then
    echo "$zero test: Failed"
    exit -1
else
	echo "$zero test: Passed!"
fi


echo "Done testing!"