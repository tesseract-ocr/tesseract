import type { ImageLoaderProps } from './image-config';
import type { ImageProps, ImageLoader, StaticImageData } from './get-img-props';
import { Image } from '../../client/image-component';
/**
 * For more advanced use cases, you can call `getImageProps()`
 * to get the props that would be passed to the underlying `<img>` element,
 * and instead pass to them to another component, style, canvas, etc.
 *
 * Read more: [Next.js docs: `getImageProps`](https://nextjs.org/docs/app/api-reference/components/image#getimageprops)
 */
export declare function getImageProps(imgProps: ImageProps): {
    props: import("./get-img-props").ImgProps;
};
export default Image;
export type { ImageProps, ImageLoaderProps, ImageLoader, StaticImageData };
