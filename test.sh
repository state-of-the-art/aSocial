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

# Script run unit tests and does plugins standalone validation for asocial using docker qt image

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

"$BASEDIR/check.sh"

# Build the entire project
docker run -i --rm \
    -v "${BASEDIR}:/home/user/project:ro" \
    -v "${BASEDIR}/build/downloads:/home/user/downloads:rw" \
    asocial:build \
    sh -ec '
[ -d build ] || mkdir -p build
sudo chmod -R o+rwX ./downloads ./build
qt-cmake ./project -G Ninja -B ./build -DBUILD_TESTS=ON -DLIBS_DOWNLOAD_CACHE_DIR=/home/user/downloads
cmake --build ./build

echo
echo =========== RUNNING TESTS ===========
find ./build/plugins -name "DartConfiguration.tcl" -execdir ctest -V \;
'

# Make sure each plugin can be built separately
plugins=''
plugins_count=0
for plugin_path in `find ${BASEDIR}/plugins -mindepth 1 -maxdepth 1 -type d`; do
    [ -f "$plugin_path/CMakeLists.txt" ] || continue
    plugin=$(basename "$plugin_path")
    echo
    echo "Checking standalone $plugin build"
    plugins_count=$(($plugins_count+1))
    plugins="$plugins $plugin"
    docker run -i --rm \
        -v "${BASEDIR}:/home/user/project:ro" \
        -v "${BASEDIR}/build/downloads:/home/user/downloads:rw" \
        asocial:build \
        sh -ec "
[ -d build ] || mkdir -p build
sudo chmod -R o+rwX ./downloads ./build
qt-cmake ./project/plugins/$plugin -G Ninja -B ./build -DBUILD_TESTS=ON -DLIBS_DOWNLOAD_CACHE_DIR=/home/user/downloads
cmake --build ./build
"
done

echo
echo "Successfully checked $plugins_count plugins: $plugins"
