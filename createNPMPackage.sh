#!/bin/bash
set -e
set -x

ROOT=$(pwd)

unset CI

react_native_major_version="71"
react_native_version="0.71"

rm -rf node_modules
yarn --frozen-lockfile

for for_hermes in "True" "False"
do
  engine="jsc"
  if [ "$for_hermes" == "True" ]; then
    engine="hermes"
  fi
  echo "engine=${engine}"

  cd android
  ./gradlew clean

  CLIENT_SIDE_BUILD="False" JS_RUNTIME=${engine} REANIMATED_PACKAGE_BUILD="1" ./gradlew :assembleDebug --no-build-cache --rerun-tasks

  cd $ROOT

  rm -rf android/react-native-reanimated-"$react_native_major_version-${engine}".aar
  cp android/build/outputs/aar/*.aar android/react-native-reanimated-"$react_native_major_version-${engine}".aar
done

# Hack to make tsc work. In the upstream repo, it worked because it was using the type signatures
# of an old RN version (0.71.0-rc.5)
# This can probably be removed in the next reanimated version
patch node_modules/react-native/Libraries/StyleSheet/StyleSheetTypes.d.ts fix-rn-types.patch

cp -R android/build build_output
cd android && REANIMATED_PACKAGE_BUILD="1" ./gradlew clean && cd ..
yarn run type:generate
npm pack

rm -rf ./lib

echo "Done!"
