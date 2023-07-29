# Taken from pcsx-redux create-app-bundle.sh
# For Plist buddy
PATH="$PATH:/usr/libexec"

chmod +r docs/img/icon.ico
# Construct the app iconset.
mkdir chonkystation.iconset
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 72 -resize 16x16 chonkystation.iconset/icon_16x16.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 144 -resize 32x32 chonkystation.iconset/icon_16x16@2x.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 72 -resize 32x32 chonkystation.iconset/icon_32x32.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 144 -resize 64x64 chonkystation.iconset/icon_32x32@2x.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 72 -resize 128x128 chonkystation.iconset/icon_128x128.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 144 -resize 256x256 chonkystation.iconset/icon_128x128@2x.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 72 -resize 256x256 chonkystation.iconset/icon_256x256.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 144 -resize 512x512 chonkystation.iconset/icon_256x256@2x.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 72 -resize 512x512 chonkystation.iconset/icon_512x512.png
convert docs/img/icon.ico -alpha off -background none -units PixelsPerInch -density 144 -resize 1024x1024 chonkystation.iconset/icon_512x512@2x.png
iconutil --convert icns chonkystation.iconset

# Set up the .app directory
mkdir -p ChonkyStation.app/Contents/MacOS/Libraries
mkdir ChonkyStation.app/Contents/Resources


# Copy binary into App
cp ./build/ChonkyStation ChonkyStation.app/Contents/MacOS/ChonkyStation
chmod a+x ChonkyStation.app/Contents/Macos/ChonkyStation

# Copy icons into App
cp chonkystation.icns ChonkyStation.app/Contents/Resources/AppIcon.icns

# Fix up Plist stuff
PlistBuddy ChonkyStation.app/Contents/Info.plist -c "add CFBundleDisplayName string ChonkyStation"
PlistBuddy ChonkyStation.app/Contents/Info.plist -c "add CFBundleIconName string AppIcon"
PlistBuddy ChonkyStation.app/Contents/Info.plist -c "add CFBundleIconFile string AppIcon"
PlistBuddy ChonkyStation.app/Contents/Info.plist -c "add NSHighResolutionCapable bool true"
PlistBuddy ChonkyStation.app/Contents/version.plist -c "add ProjectName string ChonkyStation"

# Bundle dylibs
dylibbundler -od -b -x ChonkyStation.app/Contents/MacOS/ChonkyStation -d ChonkyStation.app/Contents/Frameworks/

# relative rpath
install_name_tool -add_rpath @loader_path/../Frameworks ChonkyStation.app/Contents/MacOS/ChonkyStation