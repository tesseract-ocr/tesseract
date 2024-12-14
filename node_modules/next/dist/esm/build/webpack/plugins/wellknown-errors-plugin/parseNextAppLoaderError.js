import { relative } from 'path';
import { SimpleWebpackError } from './simpleWebpackError';
export function getNextAppLoaderError(err, module, compiler) {
    try {
        if (!module.loaders[0].loader.includes('next-app-loader')) {
            return false;
        }
        const file = relative(compiler.context, module.buildInfo.route.absolutePagePath);
        return new SimpleWebpackError(file, err.message);
    } catch  {
        return false;
    }
}

//# sourceMappingURL=parseNextAppLoaderError.js.map