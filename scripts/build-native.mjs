#!/usr/bin/env zx


let rootDir = await path.resolve(__dirname, '..');
let buildDir = await path.join(rootDir, 'native', 'build', 'scripted');

echo(`Root directory: ${rootDir}`);
echo(`Build directory: ${buildDir}`);

await fs.ensureDir(buildDir);

cd(buildDir);

await $`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./out/ -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 ../..`;
await $`cmake --build . --config Release -j 4`;
await $`cmake --install . --component plugin`;
