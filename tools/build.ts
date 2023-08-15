import {build} from 'esbuild';
import {buildWasm} from "./build-wasm.js";
import {writeFileSync, copyFileSync, readFileSync, mkdirSync} from "fs";
import {execSync} from "child_process";
// import {config} from "@swc/core";
// import {bundle, minify, transformFile} from "@swc/core";

try {
    mkdirSync("build/public", {recursive: true});
} catch {
}

buildWasm();

copyFileSync("wasm/build/lib.wasm", "build/public/lib.wasm");
copyFileSync("client/assets/index.html", "build/public/index.html");

// const b = await bundle(
// {
//     mode: "production",
//     target: "browser",
//     workingDir: process.cwd(),
//     entry: {
//         'web': "./client/src/index.ts",
//     },
//     output: {
//         name: "web",
//         path: "./build/_index.js"
//     },
//     module: {},
// });
// // b.web.code = (await minify(b.web.code)).code;
// writeFileSync("build/_index.js", b.web.code);

const buildTasks = [
    build({
        entryPoints: ["client/src/index.ts"],
        outfile: "build/public/index.js",
        tsconfig: "client/tsconfig.json",
        bundle: true,
        minifySyntax: true,
        format: "esm",
        plugins: []
    }).catch(e => {
        console.warn(e);
        process.exit(1);
    }),
    build({
        entryPoints: ["server/src/index.ts"],
        outfile: "build/index.js",
        tsconfig: "server/tsconfig.json",
        bundle: true,
        minifySyntax: true,
        platform: "node",
        target: "node16",
        format: "esm",
    }).catch(e => {
        console.warn(e);
        process.exit(1);
    })
]

await Promise.all(buildTasks);

function sz(...files: string[]) {
    let total = 0;
    for (const file of files) {
        try {
            const bytes = readFileSync(file);
            total += bytes.length;
        } catch {
            console.warn("file not found:", file);
        }
    }
    return total;
}

const zip = (label: string, ...files: string[]) => {
    try {
        const zipname = "build/" + label + ".zip";
        console.log("estimate zip...");
        execSync(`advzip --shrink-insane --iter=1000 --add ${zipname} ${files.join(" ")}`);

        console.info("files: " + sz(...files));
        console.info("zip: " + sz(zipname));
    } catch {
        console.warn("zip not found. please install `advancecomp`");
    }
};

zip("game", "build/public/index.js", "build/public/index.html", "build/public/lib.wasm");

execSync("node index.js", {cwd: "build", stdio: "inherit"});

