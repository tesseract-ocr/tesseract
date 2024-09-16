import type { ResolvedMetadata, Viewport } from '../types/metadata-interface';
import type { FieldResolver, FieldResolverExtraArgs, MetadataContext } from '../types/resolvers';
export declare const resolveThemeColor: FieldResolver<'themeColor', Viewport>;
export declare const resolveAlternates: FieldResolverExtraArgs<'alternates', [
    ResolvedMetadata['metadataBase'],
    MetadataContext
]>;
export declare const resolveRobots: FieldResolver<'robots'>;
export declare const resolveVerification: FieldResolver<'verification'>;
export declare const resolveAppleWebApp: FieldResolver<'appleWebApp'>;
export declare const resolveAppLinks: FieldResolver<'appLinks'>;
export declare const resolveItunes: FieldResolverExtraArgs<'itunes', [
    ResolvedMetadata['metadataBase'],
    MetadataContext
]>;
export declare const resolveFacebook: FieldResolver<'facebook'>;
