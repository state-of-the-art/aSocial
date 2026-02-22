#!/bin/sh -e
# Script builds asocial using docker qt image

BASEDIR=$(dirname `readlink -f $0`)

# Check if we have no image for the build
if [ ! "$(docker images -q asocial:build)" ]; then
    docker build -t asocial:build -f "$BASEDIR/docker/Dockerfile.build" "$BASEDIR/docker"
else
    # Check if the image is fresh in comparison to the dockerfile
    docker_image_ts=$(date +%s --date "$(docker image inspect --format "{{.Created}}" asocial:build)")
    dockerfile_ts=$(stat -c '%Y' "$BASEDIR/docker/Dockerfile.build")
    if [ "$docker_image_ts" -lt "$dockerfile_ts" ]; then
        docker build -t asocial:build -f "$BASEDIR/docker/Dockerfile.build" "$BASEDIR/docker"
    fi
fi

#    -v "${BASEDIR}/build/docker:/home/user/build:rw" \

# Build the project
docker run -i --rm \
    -v "${BASEDIR}:/home/user/project:ro" \
    -v "${BASEDIR}/build/downloads:/home/user/downloads:rw" \
    -v "${PWD}:/home/user/out:rw" \
    asocial:build \
    sh -ec '
[ -d build ] || mkdir -p build
sudo chmod -R o+rwX ./downloads ./build
qt-cmake ./project -G Ninja -B ./build -DCMAKE_BUILD_TYPE=Release -DLIBS_DOWNLOAD_CACHE_DIR=/home/user/downloads
cmake --build ./build

# Adding wayland into mix
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"

# Excluding sql drivers we dont need (adds dependencies and complexity)
export LINUXDEPLOY_EXCLUDED_LIBRARIES="libqsql*.so"

linuxdeploy --appdir ./deploy --plugin qt \
  -e "$(find ./build -maxdepth 1 -type f -executable)" \
  $(find ./build/plugins -maxdepth 1 -type f -name "libasocial-plugin-*.so" -printf "-l %p ") \
  -d ./project/asocial.desktop \
  -i ./project/asocial.icon.svg --icon-filename asocial

# Adding symlinks to plugins into app share directory
mkdir -p ./deploy/usr/share/asocial/plugins
for f in `ls ./deploy/usr/lib/libasocial-plugin-*`; do
    name="$(basename "$f")"
    echo "Linking plugin $name"
    ln -s "../../../lib/$name" "./deploy/usr/share/asocial/plugins/$name"
done

# Completing the appimage
linuxdeploy --appdir ./deploy --output appimage

sudo mv aSocial-x86_64.AppImage ./out/
'

# Changing ownership of the resulting file to current user
cp -a aSocial-x86_64.AppImage tmp-aSocial-x86_64.AppImage
rm -f aSocial-x86_64.AppImage
mv tmp-aSocial-x86_64.AppImage aSocial-x86_64.AppImage
