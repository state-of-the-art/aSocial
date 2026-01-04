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

# Build the project
docker run -i --rm \
    -v "${BASEDIR}:/home/user/project:ro" \
    -v "${BASEDIR}/build/downloads:/home/user/downloads:rw" \
    -v "${PWD}:/home/user/out:rw" \
    asocial:build \
    sh -ec '
sudo chmod -R o+rwX ./downloads
qt-cmake ./project -G Ninja -B ./build -DLIBS_DOWNLOAD_CACHE_DIR=/home/user/downloads
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
