#!/bin/sh -e
# Script builds asocial using docker qt image

BASEDIR=$(dirname `readlink -f $0`)

docker run -i --rm -v "${BASEDIR}:/home/user/project:ro" -v "${PWD}:/home/user/out:rw" stateoftheartio/qt6:6.10-gcc-aqt \
    sh -ec '
sudo apt update
sudo apt install -y libgl-dev libvulkan-dev pkg-config file

qt-cmake ./project -G Ninja -B ./build
cmake --build ./build

linuxdeploy --appdir ./deploy --plugin qt \
  -e "$(find ./build -maxdepth 1 -type f -executable)" \
  -d ./project/asocial.desktop \
  -i ./project/asocial.icon.svg --icon-filename asocial

# Adding plugins to share directory
mkdir -p ./deploy/usr/share/asocial/plugins
for f in ./build/plugins/libasocial-plugin-*; do
    name="$(basename "$f")"
    echo "Copy plugin $name"
    cp -a "$f" "./deploy/usr/share/asocial/plugins/$name"
done

# Completing the appimage
linuxdeploy --appdir ./deploy --output appimage

sudo mv aSocial-x86_64.AppImage ./out/
'

# Changing ownership of the resulting file to current user
cp -a aSocial-x86_64.AppImage tmp-aSocial-x86_64.AppImage
rm -f aSocial-x86_64.AppImage
mv tmp-aSocial-x86_64.AppImage aSocial-x86_64.AppImage
