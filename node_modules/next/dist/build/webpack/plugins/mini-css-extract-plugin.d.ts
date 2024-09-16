import MiniCssExtractPlugin from 'next/dist/compiled/mini-css-extract-plugin';
export default class NextMiniCssExtractPlugin extends MiniCssExtractPlugin {
    __next_css_remove: boolean;
}
