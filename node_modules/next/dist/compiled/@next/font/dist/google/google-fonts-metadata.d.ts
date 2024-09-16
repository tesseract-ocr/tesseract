type GoogleFontsMetadata = {
    [fontFamily: string]: {
        weights: string[];
        styles: string[];
        subsets: string[];
        axes?: Array<{
            tag: string;
            min: number;
            max: number;
            defaultValue: number;
        }>;
    };
};
export declare const googleFontsMetadata: GoogleFontsMetadata;
export {};
