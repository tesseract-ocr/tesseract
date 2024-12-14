export default async function optimize(html, config) {
    let AmpOptimizer;
    try {
        AmpOptimizer = require('next/dist/compiled/@ampproject/toolbox-optimizer');
    } catch (_) {
        return html;
    }
    const optimizer = AmpOptimizer.create(config);
    return optimizer.transformHtml(html, config);
}

//# sourceMappingURL=optimize-amp.js.map