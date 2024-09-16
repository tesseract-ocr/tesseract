export type AppLinks = {
    ios?: AppLinksApple | Array<AppLinksApple>;
    iphone?: AppLinksApple | Array<AppLinksApple>;
    ipad?: AppLinksApple | Array<AppLinksApple>;
    android?: AppLinksAndroid | Array<AppLinksAndroid>;
    windows_phone?: AppLinksWindows | Array<AppLinksWindows>;
    windows?: AppLinksWindows | Array<AppLinksWindows>;
    windows_universal?: AppLinksWindows | Array<AppLinksWindows>;
    web?: AppLinksWeb | Array<AppLinksWeb>;
};
export type ResolvedAppLinks = {
    ios?: Array<AppLinksApple>;
    iphone?: Array<AppLinksApple>;
    ipad?: Array<AppLinksApple>;
    android?: Array<AppLinksAndroid>;
    windows_phone?: Array<AppLinksWindows>;
    windows?: Array<AppLinksWindows>;
    windows_universal?: Array<AppLinksWindows>;
    web?: Array<AppLinksWeb>;
};
export type AppLinksApple = {
    url: string | URL;
    app_store_id?: string | number;
    app_name?: string;
};
export type AppLinksAndroid = {
    package: string;
    url?: string | URL;
    class?: string;
    app_name?: string;
};
export type AppLinksWindows = {
    url: string | URL;
    app_id?: string;
    app_name?: string;
};
export type AppLinksWeb = {
    url: string | URL;
    should_fallback?: boolean;
};
export type ItunesApp = {
    appId: string;
    appArgument?: string;
};
export type ViewportLayout = {
    width?: string | number;
    height?: string | number;
    initialScale?: number;
    minimumScale?: number;
    maximumScale?: number;
    userScalable?: boolean;
    viewportFit?: 'auto' | 'cover' | 'contain';
    interactiveWidget?: 'resizes-visual' | 'resizes-content' | 'overlays-content';
};
export type AppleWebApp = {
    capable?: boolean;
    title?: string;
    startupImage?: AppleImage | Array<AppleImage>;
    statusBarStyle?: 'default' | 'black' | 'black-translucent';
};
export type AppleImage = string | AppleImageDescriptor;
export type AppleImageDescriptor = {
    url: string;
    media?: string;
};
export type ResolvedAppleWebApp = {
    capable: boolean;
    title?: string | null;
    startupImage?: AppleImageDescriptor[] | null;
    statusBarStyle?: 'default' | 'black' | 'black-translucent';
};
export type Facebook = FacebookAppId | FacebookAdmins;
export type FacebookAppId = {
    appId: string;
    admins?: never;
};
export type FacebookAdmins = {
    appId?: never;
    admins: string | string[];
};
export type ResolvedFacebook = {
    appId?: string;
    admins?: string[];
};
export type FormatDetection = {
    telephone?: boolean;
    date?: boolean;
    address?: boolean;
    email?: boolean;
    url?: boolean;
};
