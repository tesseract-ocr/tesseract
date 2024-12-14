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
    width?: number | `${number}`;
    height?: number | `${number}`;
    fill?: boolean;
    loader?: import("../shared/lib/get-img-props").ImageLoader;
    quality?: number | `${number}`;
    priority?: boolean;
    loading?: "eager" | "lazy" | undefined;
    placeholder?: PlaceholderValue;
    blurDataURL?: string;
    unoptimized?: boolean;
    overrideSrc?: string;
    onLoadingComplete?: OnLoadingComplete;
    layout?: string;
    objectFit?: string;
    objectPosition?: string;
    lazyBoundary?: string;
    lazyRoot?: string;
} & React.RefAttributes<HTMLImageElement | null>>;
