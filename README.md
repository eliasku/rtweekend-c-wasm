# Ray Tracing in One Weekend
------
Using minimalistic C and WebAssembly

<p align="center">
<img width="600" src="./images/final.jpg" alt="final render">
</p>

## Build & Run

Provide environment variables for LLVM and Binaryen if not match default locations

```shell
export LLVM_ROOT="/usr/local/opt/llvm"
export BINARYEN_ROOT="/usr/local/opt/binaryen"
```

Install dependencies, build and start local server

```shell
npm i
npm start
```

Navigate to [localhost:8080](http://localhost:8080)

## Target goals

- Use pure C language
- No Emscripten
- No `stdlib`
- No heap allocations
- Compile to `wasm32` using clang
