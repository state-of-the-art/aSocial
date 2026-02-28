#/bin/sh
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

# Script to simplify the style check process

root_dir=$(realpath "$(dirname "$0")")
errors=0

echo
echo '---------------------- Custom Checks ----------------------'
echo

# Checking only added/modified files since master
for f in $(git diff --name-only origin/master); do
    # Check text files
    if file "$f" | grep -q 'text'; then
        # Ends with newline as POSIX requires
        if [ -n "$(tail -c 1 "$f")" ]; then
            echo "ERROR: Should end with newline: $f"
            errors=$((${errors} + 1))
        fi

        # Logic files: go, proto, sh
        if echo "$f" | grep -q '\.\(cpp\|h\|sh\|cmake\)\|CMakeLists.txt$'; then
            if echo "$f" | fgrep -q '.gen.'; then
                continue
            fi
            if head -20 "$f" | grep -q 'Code generated.* DO NOT EDIT.'; then
                continue
            fi

            # Should contain copyright
            if !(head -20 "$f" | grep -q 'Copyright (C) 20.\+  aSocial Developers'); then
                echo "ERROR: Should contain aSocial Developers copyright header: $f"
                errors=$((${errors} + 1))
            fi

            # Should contain license
            if !(head -20 "$f" | fgrep -q 'GNU General Public License'); then
                echo "ERROR: Should contain license name: $f"
                errors=$((${errors} + 1))
            fi

            #  Should contain Author
            if !(head -20 "$f" | grep -q 'Author: .\+'); then
                echo "ERROR: Should contain Author: $f"
                errors=$((${errors} + 1))
            fi

            # Copyright year in modified files should be the current year
            if !(head -20 "$f" | grep 'Copyright (C) 20.\+  aSocial Developers' | fgrep -q "$(date '+%Y')"); then
                echo "ERROR: Copyright header need to be adjusted to contain current year like: 20??-$(date '+%Y') $f"
                errors=$((${errors} + 1))
            fi
        fi
    fi
done

# Check for spaces in the end of the line
blank_char_end=$(git grep -lI '[[:blank:]]$')
if [ "${blank_char_end}" ]; then
    echo "ERROR: Please fix line end blank chars in: \n${blank_char_end}"
    errors=$((${errors} + $(echo "${blank_char_end}" | wc -l)))
fi

echo
echo '---------------------- clang-format verify ----------------------'
echo
reformat=$(git clang-format origin/master --quiet --diff)
if [ "${reformat}" ]; then
    echo "ERROR: Please run 'git clang-format': \n${reformat}"
    errors=$((${errors} + $(echo "${reformat}" | wc -l)))
fi

exit ${errors}
