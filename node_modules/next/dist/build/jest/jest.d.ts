import type { Config } from '@jest/types';
/**
 * @example
 * ```ts
 * // Usage in jest.config.js
 * const nextJest = require('next/jest');
 *
 * // Optionally provide path to Next.js app which will enable loading next.config.js and .env files
 * const createJestConfig = nextJest({ dir })
 *
 * // Any custom config you want to pass to Jest
 * const customJestConfig = {
 *     setupFilesAfterEnv: ['<rootDir>/jest.setup.js'],
 * }
 *
 * // createJestConfig is exported in this way to ensure that next/jest can load the Next.js config which is async
 * module.exports = createJestConfig(customJestConfig)
 * ```
 *
 * Read more: [Next.js Docs: Setting up Jest with Next.js](https://nextjs.org/docs/app/building-your-application/testing/jest)
 */
export default function nextJest(options?: {
    dir?: string;
}): (customJestConfig?: Config.InitialProjectOptions | (() => Promise<Config.InitialProjectOptions>)) => () => Promise<Config.InitialProjectOptions>;
