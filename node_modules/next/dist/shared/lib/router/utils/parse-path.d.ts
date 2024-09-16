/**
 * Given a path this function will find the pathname, query and hash and return
 * them. This is useful to parse full paths on the client side.
 * @param path A path to parse e.g. /foo/bar?id=1#hash
 */
export declare function parsePath(path: string): {
    pathname: string;
    query: string;
    hash: string;
};
