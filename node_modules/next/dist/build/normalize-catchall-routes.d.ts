import { AppPathnameNormalizer } from '../server/normalizers/built/app/app-pathname-normalizer';
/**
 * This function will transform the appPaths in order to support catch-all routes and parallel routes.
 * It will traverse the appPaths, looking for catch-all routes and try to find parallel routes that could match
 * the catch-all. If it finds a match, it will add the catch-all to the parallel route's list of possible routes.
 *
 * @param appPaths The appPaths to transform
 */
export declare function normalizeCatchAllRoutes(appPaths: Record<string, string[]>, normalizer?: AppPathnameNormalizer): void;
