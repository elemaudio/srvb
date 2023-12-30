#!/usr/bin/env zx


let rootDir = await path.resolve(__dirname, '..');
let buildDir = await path.join(rootDir, 'native', 'build', 'scripted');

echo(`Root directory: ${rootDir}`);
echo(`Build directory: ${buildDir}`);

// Clean the build directory before we build
await fs.remove(buildDir);
await fs.ensureDir(buildDir);

cd(buildDir);

let buildType = argv.dev ? 'Debug' : 'Release';
let devFlag = argv.dev ? '-DELEM_DEV_LOCALHOST=1' : '';

await $`cmake -DCMAKE_BUILD_TYPE=${buildType} -DCMAKE_INSTALL_PREFIX=./out/ -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 ${devFlag} ../..`;
await $`cmake --build . --config ${buildType} -j 4`;
