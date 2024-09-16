type OgModule = typeof import('next/dist/compiled/@vercel/og');
/**
 * The ImageResponse class allows you to generate dynamic images using JSX and CSS.
 * This is useful for generating social media images such as Open Graph images, Twitter cards, and more.
 *
 * Read more: [Next.js Docs: `ImageResponse`](https://nextjs.org/docs/app/api-reference/functions/image-response)
 */
export declare class ImageResponse extends Response {
    static displayName: string;
    constructor(...args: ConstructorParameters<OgModule['ImageResponse']>);
}
export {};
