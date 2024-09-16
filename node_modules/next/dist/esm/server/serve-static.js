import send from "next/dist/compiled/send";
// TODO: Remove this once "send" has updated the "mime", or next.js use custom version of "mime"
// Although "mime" has already add avif in version 2.4.7, "send" is still using mime@1.6.0
send.mime.define({
    "image/avif": [
        "avif"
    ]
});
export function serveStatic(req, res, path, opts) {
    return new Promise((resolve, reject)=>{
        send(req, path, opts).on("directory", ()=>{
            // We don't allow directories to be read.
            const err = new Error("No directory access");
            err.code = "ENOENT";
            reject(err);
        }).on("error", reject).pipe(res).on("finish", resolve);
    });
}
export const getContentType = "getType" in send.mime ? (extWithoutDot)=>send.mime.getType(extWithoutDot) : (extWithoutDot)=>send.mime.lookup(extWithoutDot);
export const getExtension = "getExtension" in send.mime ? (contentType)=>send.mime.getExtension(contentType) : (contentType)=>send.mime.extension(contentType);

//# sourceMappingURL=serve-static.js.map