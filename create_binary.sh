#!/usr/bin/env bash
set -e

#First find the folder, where this script resides.
SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

#We expect subfolder cpp there.
cd $DIR/cpp

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build  --verbose

echo ""
echo ""
echo "Everything is prepared. You can start EDMC now."

