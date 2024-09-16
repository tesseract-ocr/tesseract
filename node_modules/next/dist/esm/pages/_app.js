import { jsx as _jsx } from "react/jsx-runtime";
import React from "react";
import { loadGetInitialProps } from "../shared/lib/utils";
/**
 * `App` component is used for initialize of pages. It allows for overwriting and full control of the `page` initialization.
 * This allows for keeping state between navigation, custom error handling, injecting additional data.
 */ async function appGetInitialProps(param) {
    let { Component, ctx } = param;
    const pageProps = await loadGetInitialProps(Component, ctx);
    return {
        pageProps
    };
}
class App extends React.Component {
    render() {
        const { Component, pageProps } = this.props;
        return /*#__PURE__*/ _jsx(Component, {
            ...pageProps
        });
    }
}
App.origGetInitialProps = appGetInitialProps;
App.getInitialProps = appGetInitialProps;
export { App as default };

//# sourceMappingURL=_app.js.map