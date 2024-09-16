/// <reference types="react" />
import type { ImageConfigComplete, ImageLoaderProps, ImageLoaderPropsWithConfig } from './image-config';
export interface StaticImageData {
    src: string;
    height: number;
    width: number;
    blurDataURL?: string;
    blurWidth?: number;
    blurHeight?: number;
}
export interface StaticRequire {
    default: StaticImageData;
}
export type StaticImport = StaticRequire | StaticImageData;
export type ImageProps = Omit<JSX.IntrinsicElements['img'], 'src' | 'srcSet' | 'ref' | 'alt' | 'width' | 'height' | 'loading'> & {
    src: string | StaticImport;
    alt: string;
    width?: number | `${number}`;
    height?: number | `${number}`;
    fill?: boolean;
    loader?: ImageLoader;
    quality?: number | `${number}`;
    priority?: boolean;
    loading?: LoadingValue;
    placeholder?: PlaceholderValue;
    blurDataURL?: string;
    unoptimized?: boolean;
    overrideSrc?: string;
    /**
     * @deprecated Use `onLoad` instead.
     * @see https://nextjs.org/docs/app/api-reference/components/image#onload
     */
    onLoadingComplete?: OnLoadingComplete;
    /**
     * @deprecated Use `fill` prop instead of `layout="fill"` or change import to `next/legacy/image`.
     * @see https://nextjs.org/docs/api-reference/next/legacy/image
     */
    layout?: string;
    /**
     * @deprecated Use `style` prop instead.
     */
    objectFit?: string;
    /**
     * @deprecated Use `style` prop instead.
     */
    objectPosition?: string;
    /**
     * @deprecated This prop does not do anything.
     */
    lazyBoundary?: string;
    /**
     * @deprecated This prop does not do anything.
     */
    lazyRoot?: string;
};
export type ImgProps = Omit<ImageProps, 'src' | 'alt' | 'loader'> & {
    loading: LoadingValue;
    width: number | undefined;
    height: number | undefined;
    style: NonNullable<JSX.IntrinsicElements['img']['style']>;
    sizes: string | undefined;
    srcSet: string | undefined;
    src: string;
};
declare const VALID_LOADING_VALUES: readonly ["lazy", "eager", undefined];
type LoadingValue = (typeof VALID_LOADING_VALUES)[number];
export type ImageLoader = (p: ImageLoaderProps) => string;
type ImageLoaderWithConfig = (p: ImageLoaderPropsWithConfig) => string;
export type PlaceholderValue = 'blur' | 'empty' | `data:image/${string}`;
export type OnLoad = React.ReactEventHandler<HTMLImageElement> | undefined;
export type OnLoadingComplete = (img: HTMLImageElement) => void;
/**
 * A shared function, used on both client and server, to generate the props for <img>.
 */
export declare function getImgProps({ src, sizes, unoptimized, priority, loading, className, quality, width, height, fill, style, overrideSrc, onLoad, onLoadingComplete, placeholder, blurDataURL, fetchPriority, layout, objectFit, objectPosition, lazyBoundary, lazyRoot, ...rest }: ImageProps, _state: {
    defaultLoader: ImageLoaderWithConfig;
    imgConf: ImageConfigComplete;
    showAltText?: boolean;
    blurComplete?: boolean;
}): {
    props: ImgProps;
    meta: {
        unoptimized: boolean;
        priority: boolean;
        placeholder: NonNullable<ImageProps['placeholder']>;
        fill: boolean;
    };
};
export {};
