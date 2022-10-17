import {createServer, IncomingMessage, OutgoingHttpHeaders, ServerResponse} from "http";
import {readFile} from "fs";

const MIME_TYPES_1: Record<string, OutgoingHttpHeaders> = {
    n: {
        "content-type": "application/json"
    },
    l: {
        "content-type": "text/html;charset=utf-8",
        "cache-control": "no-cache",
    },
    s: {
        "cache-control": "no-cache"
    },
    f: {
        "content-type": "font/ttf",
        "cache-control": "max-age=86400"
    },
    m: {
        "content-type": "application/wasm",
        "cache-control": "no-cache"
    },
};

const serveStatic = (file: string, res: ServerResponse, mime: OutgoingHttpHeaders) =>
    readFile(
        "./public" + file,
        (err, data) => {
            res.writeHead(err ? 404 : 200, mime);
            res.end(data);
        }
    );

const error = (req: IncomingMessage, res: ServerResponse) => {
    res.writeHead(500);
    res.end();
}

createServer((req: IncomingMessage, res: ServerResponse) => {
    if(req.method === "GET") {
        if(req.url === "/") {
            req.url = "/index.html";
        }
        serveStatic(req.url, res, MIME_TYPES_1[req.url.at(-1)]);
    }
    else {
        error(req, res);
    }
}).listen(+process.env.PORT || 8080);

// console will be dropped for prod build
console.log("Local server http://localhost:8080");