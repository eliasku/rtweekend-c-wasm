import {execSync, ExecSyncOptions} from "child_process";
import {mkdirSync} from "fs";

export const buildWasm = () => {
    const LLVM_PATH = "/usr/local/opt/llvm";
    const BINARYEN_PATH = "/Users/ilyak/dev/binaryen";

    const env = Object.assign({}, process.env, {
        PATH: `${LLVM_PATH}/bin:${BINARYEN_PATH}/bin:${process.env.PATH}`,
        LDFLAGS: `-L${LLVM_PATH}/lib`,
        CPPFLAGS: `-I${LLVM_PATH}/include`,
        CFLAGS: `-I${LLVM_PATH}/include`
    });

    const RELINK_FLAGS = "-Oz";

    const PAGE_SIZE = 65536;
    const STACK_SIZE = 2 * PAGE_SIZE;
    const TOTAL_MEMORY = 64 * PAGE_SIZE;
    const COMPILE_FLAGS = `-I. -Wall --target=wasm32 -Oz -nostdlib -fvisibility=hidden -std=c11 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -ffast-math -flto`;
    const LINK_FLAGS = `--export=__heap_base --no-entry --strip-all --export-dynamic --allow-undefined --initial-memory=${TOTAL_MEMORY} -z stack-size=${STACK_SIZE} --error-limit=0 --lto-O3 -O3 --gc-sections`;

    try {
        mkdirSync("wasm/build");
    }
    catch {}

    const opts:ExecSyncOptions = {env, stdio: "inherit", cwd: "./wasm", encoding: "utf8"};
    execSync(`clang -c ${COMPILE_FLAGS} -o build/lib.o src/lib.c`, opts);
    execSync(`wasm-ld ${LINK_FLAGS} -o build/lib-raw.wasm build/lib.o`, opts);
    execSync(`wasm-opt ${RELINK_FLAGS} build/lib-raw.wasm -o build/lib.wasm`,opts);
};
