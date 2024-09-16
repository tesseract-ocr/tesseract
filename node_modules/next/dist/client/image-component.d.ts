import React from 'react';
import type { OnLoadingComplete, PlaceholderValue } from '../shared/lib/get-img-props';
import type { ImageLoaderProps } from '../shared/lib/image-config';
export type { ImageLoaderProps };
export type ImageLoader = (p: ImageLoaderProps) => string;
/**
 * The `Image` component is used to optimize images.
 *
 * Read more: [Next.js docs: `Image`](https://nextjs.org/docs/app/api-reference/components/image)
 */
export declare const Image: React.ForwardRefExoticComponent<Omit<React.DetailedHTMLProps<React.ImgHTMLAttributes<HTMLImageElement>, HTMLImageElement>, "height" | "width" | "loading" | "ref" | "alt" | "src" | "srcSet"> & {
    src: string | import("../shared/lib/get-img-props").StaticImport;
    alt: string;
    width?: number | `${number}` | undefined;
    height?: number | `${number}` | undefined;
    fill?: boolean | undefined;
    loader?: import("../shared/lib/get-img-props").ImageLoader | undefined;
    quality?: number | `${number}` | undefined;
    priority?: boolean | undefined;
    loading?: "eager" | "lazy" | undefined;
    placeholder?: PlaceholderValue | undefined;
    blurDataURL?: string | undefined;
    unoptimized?: boolean | undefined;
    overrideSrc?: string | undefined;
    onLoadingComplete?: OnLoadingComplete | undefined;
    layout?: string | undefined;
    objectFit?: string | undefined;
    objectPosition?: string | undefined;
    lazyBoundary?: string | undefined;
    lazyRoot?: string | undefined;
} & React.RefAttributes<HTMLImageElement | null>>;
