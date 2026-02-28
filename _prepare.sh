#!/bin/sh -e
# Copyright (C) 2026  aSocial Developers
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Author: Rabit (@rabits)

# Script builds complete release asocial appimage using docker qt image

BASEDIR=$(dirname `readlink -f $0`)

# Check if we have no image for the build
if [ ! "$(docker images -q asocial:build)" ]; then
    docker build -t asocial:build -f "$BASEDIR/docker/Dockerfile.build" "$BASEDIR/docker"
else
    # Check if the image is fresh in comparison to the dockerfile
    docker_image_ts=$(date +%s --date "$(docker image inspect --format "{{.Created}}" asocial:build)")
    dockerfile_ts=$(stat -c '%Y' "$BASEDIR/docker/Dockerfile.build")
    if [ "$docker_image_ts" -lt "$dockerfile_ts" ]; then
        docker rmi asocial:build
        docker build -t asocial:build -f "$BASEDIR/docker/Dockerfile.build" "$BASEDIR/docker"
    fi
fi

# Common build script for docker asocial:build image with params
getBuildScript () {
    cmake_build_type="$1"
    qt_cmake_params="$2"
    afterbuild_commands="$3"

    linuxdeploy_nostrip=''
    if [ "$cmake_build_type" != 'Release' ]; then
        linuxdeploy_nostrip='export NO_STRIP=true'
    fi
    echo '[ -d build ] || mkdir -p build
sudo chmod -R o+rwX ./downloads ./build
qt-cmake ./project -G Ninja -B ./build -DCMAKE_BUILD_TYPE='"$cmake_build_type"' '"$qt_cmake_params"' -DLIBS_DOWNLOAD_CACHE_DIR=/home/user/downloads
cmake --build ./build

# Adding wayland into the mix
export EXTRA_PLATFORM_PLUGINS="libqwayland-egl.so;libqwayland-generic.so"

# Excluding sql drivers we dont need (adds dependencies and complexity), but keep sqlite
export LINUXDEPLOY_EXCLUDED_LIBRARIES="libqsqlibase.so;libqsqlmimer.so;libqsqlmysql.so;libqsqloci.so;libqsqlodbc.so;libqsqlpsql.so"

'"$linuxdeploy_nostrip"'

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

'"$afterbuild_commands"'
'
}
