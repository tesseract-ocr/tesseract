export default function NextPageDisallowReExportAllExports() {
    return {
        visitor: {
            ImportDeclaration (path) {
                if ([
                    "@next/font/local",
                    "@next/font/google",
                    "next/font/local",
                    "next/font/google"
                ].includes(path.node.source.value)) {
                    var _path_node_loc, _path_node_loc1;
                    const err = new SyntaxError(`"next/font" requires SWC although Babel is being used due to a custom babel config being present.\nRead more: https://nextjs.org/docs/messages/babel-font-loader-conflict`);
                    err.code = "BABEL_PARSE_ERROR";
                    err.loc = ((_path_node_loc = path.node.loc) == null ? void 0 : _path_node_loc.start) ?? ((_path_node_loc1 = path.node.loc) == null ? void 0 : _path_node_loc1.end) ?? path.node.loc;
                    throw err;
                }
            }
        }
    };
}

//# sourceMappingURL=next-font-unsupported.js.map