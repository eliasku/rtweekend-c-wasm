const ctx = c.getContext("2d", {
    alpha: false,
    willReadFrequently: true
});

interface Exports {
    memory: WebAssembly.Memory;
    __heap_base: number;
    initialize: Function;
    update: Function;
    render: Function;
    create_world: Function;
}

WebAssembly.instantiateStreaming(fetch("lib.wasm"), {
    env: {
        tanf: Math.tan,
        sinf: Math.sin,
        cosf: Math.cos,
    }
}).then(obj => {
    const {memory, initialize, update, render, create_world, __heap_base} = obj.instance.exports as any as Exports;
    const U8 = new Uint8Array(memory.buffer);

    b.onresize = (_: any,
                  w: number = b.clientWidth,
                  h: number = b.clientHeight,
                  s: number = devicePixelRatio
    ) => {
        c.width = w * s;
        c.height = h * s;
        c.style.width = w + "px";
        c.style.height = h + "px";
    };
    b.onresize(null);

    initialize();

    let sample_idx = 0;
    let ptr_world = __heap_base;
    let ptr_pixels = ptr_world + create_world(ptr_world);

    const raf = (ts: DOMHighResTimeStamp) => {
        update(ts / 1000);

        console.info("render sample:", sample_idx);
        const img = ctx.getImageData(0, 0, 320, 240);
        const bmp = img.data;
        render(ptr_world, img.width, img.height, sample_idx++, ptr_pixels + img.width * img.height * 4, ptr_pixels);
        bmp.set(U8.subarray(ptr_pixels, ptr_pixels + img.width * img.height * 4), 0);
        ctx.putImageData(img, 0, 0);

        requestAnimationFrame(raf);
    };
    raf(0);
});


