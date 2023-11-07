#!/usr/bin/env zx


let rootDir = await path.resolve(__dirname, '..');
let buildDir = await path.resolve(rootDir, 'native/build/scripted');
let outDir = await path.resolve(rootDir, 'native/build/out');

echo(`Root directory: ${rootDir}`);
echo(`Build directory: ${buildDir}`);

await fs.ensureDir(buildDir);
await fs.ensureDir(outDir);

cd(buildDir);

await $`cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${outDir} -DCMAKE_OSX_DEPLOYMENT_TARGET=10.15 ../..`;
await $`cmake --build . --config Release -j 4`;
await $`cmake --install . --component plugin`;
