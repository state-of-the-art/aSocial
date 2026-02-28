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

. "$BASEDIR/_prepare.sh"

"$BASEDIR/check.sh"

# Build the project
docker run -i --rm \
    -v "${BASEDIR}:/home/user/project:ro" \
    -v "${BASEDIR}/build/downloads:/home/user/downloads:rw" \
    -v "${BASEDIR}/build/docker:/home/user/build:rw" \
    -v "${PWD}:/home/user/out:rw" \
    asocial:build \
    sh -ec "$(getBuildScript 'Release' '' 'sudo cp -a aSocial-x86_64.AppImage ./out/')"

# Changing ownership of the resulting file to current user
cp -a aSocial-x86_64.AppImage tmp-aSocial-x86_64.AppImage
rm -f aSocial-x86_64.AppImage
mv tmp-aSocial-x86_64.AppImage aSocial-x86_64.AppImage
