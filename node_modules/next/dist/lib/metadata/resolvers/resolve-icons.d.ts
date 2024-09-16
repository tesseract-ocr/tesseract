import type { Icon, IconDescriptor } from '../types/metadata-types';
import type { FieldResolver } from '../types/resolvers';
export declare function resolveIcon(icon: Icon): IconDescriptor;
export declare const resolveIcons: FieldResolver<'icons'>;
