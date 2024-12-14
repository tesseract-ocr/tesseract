import React, { type JSX } from 'react';
declare const VALID_LOADING_VALUES: readonly ["lazy", "eager", undefined];
type LoadingValue = (typeof VALID_LOADING_VALUES)[number];
export type ImageLoader = (resolverProps: ImageLoaderProps) => string;
export type ImageLoaderProps = {
    src: string;
    width: number;
    quality?: number;
};
declare const VALID_LAYOUT_VALUES: readonly ["fill", "fixed", "intrinsic", "responsive", undefined];
type LayoutValue = (typeof VALID_LAYOUT_VALUES)[number];
type PlaceholderValue = 'blur' | 'empty';
type OnLoadingComplete = (result: {
    naturalWidth: number;
    naturalHeight: number;
}) => void;
type ImgElementStyle = NonNullable<JSX.IntrinsicElements['img']['style']>;
export interface StaticImageData {
    src: string;
    height: number;
    width: number;
    blurDataURL?: string;
}
interface StaticRequire {
    default: StaticImageData;
}
type StaticImport = StaticRequire | StaticImageData;
type SafeNumber = number | `${number}`;
export type ImageProps = Omit<JSX.IntrinsicElements['img'], 'src' | 'srcSet' | 'ref' | 'width' | 'height' | 'loading'> & {
    src: string | StaticImport;
    width?: SafeNumber;
    height?: SafeNumber;
    layout?: LayoutValue;
    loader?: ImageLoader;
    quality?: SafeNumber;
    priority?: boolean;
    loading?: LoadingValue;
    lazyRoot?: React.RefObject<HTMLElement | null> | null;
    lazyBoundary?: string;
    placeholder?: PlaceholderValue;
    blurDataURL?: string;
    unoptimized?: boolean;
    objectFit?: ImgElementStyle['objectFit'];
    objectPosition?: ImgElementStyle['objectPosition'];
    onLoadingComplete?: OnLoadingComplete;
};
export default function Image({ src, sizes, unoptimized, priority, loading, lazyRoot, lazyBoundary, className, quality, width, height, style, objectFit, objectPosition, onLoadingComplete, placeholder, blurDataURL, ...all }: ImageProps): import("react/jsx-runtime").JSX.Element;
export {};
