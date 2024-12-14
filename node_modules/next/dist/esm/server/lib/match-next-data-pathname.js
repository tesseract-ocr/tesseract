import { getPathMatch } from '../../shared/lib/router/utils/path-match';
const matcher = getPathMatch('/_next/data/:path*');
export function matchNextDataPathname(pathname) {
    if (typeof pathname !== 'string') return false;
    return matcher(pathname);
}

//# sourceMappingURL=match-next-data-pathname.js.map