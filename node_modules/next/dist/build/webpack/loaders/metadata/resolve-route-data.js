"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    resolveManifest: null,
    resolveRobots: null,
    resolveRouteData: null,
    resolveSitemap: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    resolveManifest: function() {
        return resolveManifest;
    },
    resolveRobots: function() {
        return resolveRobots;
    },
    resolveRouteData: function() {
        return resolveRouteData;
    },
    resolveSitemap: function() {
        return resolveSitemap;
    }
});
const _utils = require("../../../../lib/metadata/generate/utils");
function resolveRobots(data) {
    let content = '';
    const rules = Array.isArray(data.rules) ? data.rules : [
        data.rules
    ];
    for (const rule of rules){
        const userAgent = (0, _utils.resolveArray)(rule.userAgent || [
            '*'
        ]);
        for (const agent of userAgent){
            content += `User-Agent: ${agent}\n`;
        }
        if (rule.allow) {
            const allow = (0, _utils.resolveArray)(rule.allow);
            for (const item of allow){
                content += `Allow: ${item}\n`;
            }
        }
        if (rule.disallow) {
            const disallow = (0, _utils.resolveArray)(rule.disallow);
            for (const item of disallow){
                content += `Disallow: ${item}\n`;
            }
        }
        if (rule.crawlDelay) {
            content += `Crawl-delay: ${rule.crawlDelay}\n`;
        }
        content += '\n';
    }
    if (data.host) {
        content += `Host: ${data.host}\n`;
    }
    if (data.sitemap) {
        const sitemap = (0, _utils.resolveArray)(data.sitemap);
        // TODO-METADATA: support injecting sitemap url into robots.txt
        sitemap.forEach((item)=>{
            content += `Sitemap: ${item}\n`;
        });
    }
    return content;
}
function resolveSitemap(data) {
    const hasAlternates = data.some((item)=>Object.keys(item.alternates ?? {}).length > 0);
    const hasImages = data.some((item)=>{
        var _item_images;
        return Boolean((_item_images = item.images) == null ? void 0 : _item_images.length);
    });
    const hasVideos = data.some((item)=>{
        var _item_videos;
        return Boolean((_item_videos = item.videos) == null ? void 0 : _item_videos.length);
    });
    let content = '';
    content += '<?xml version="1.0" encoding="UTF-8"?>\n';
    content += '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9"';
    if (hasImages) {
        content += ' xmlns:image="http://www.google.com/schemas/sitemap-image/1.1"';
    }
    if (hasVideos) {
        content += ' xmlns:video="http://www.google.com/schemas/sitemap-video/1.1"';
    }
    if (hasAlternates) {
        content += ' xmlns:xhtml="http://www.w3.org/1999/xhtml">\n';
    } else {
        content += '>\n';
    }
    for (const item of data){
        var _item_alternates, _item_images, _item_videos;
        content += '<url>\n';
        content += `<loc>${item.url}</loc>\n`;
        const languages = (_item_alternates = item.alternates) == null ? void 0 : _item_alternates.languages;
        if (languages && Object.keys(languages).length) {
            // Since sitemap is separated from the page rendering, there's not metadataBase accessible yet.
            // we give the default setting that won't effect the languages resolving.
            for(const language in languages){
                content += `<xhtml:link rel="alternate" hreflang="${language}" href="${languages[language]}" />\n`;
            }
        }
        if ((_item_images = item.images) == null ? void 0 : _item_images.length) {
            for (const image of item.images){
                content += `<image:image>\n<image:loc>${image}</image:loc>\n</image:image>\n`;
            }
        }
        if ((_item_videos = item.videos) == null ? void 0 : _item_videos.length) {
            for (const video of item.videos){
                let videoFields = [
                    `<video:video>`,
                    `<video:title>${video.title}</video:title>`,
                    `<video:thumbnail_loc>${video.thumbnail_loc}</video:thumbnail_loc>`,
                    `<video:description>${video.description}</video:description>`,
                    video.content_loc && `<video:content_loc>${video.content_loc}</video:content_loc>`,
                    video.player_loc && `<video:player_loc>${video.player_loc}</video:player_loc>`,
                    video.duration && `<video:duration>${video.duration}</video:duration>`,
                    video.view_count && `<video:view_count>${video.view_count}</video:view_count>`,
                    video.tag && `<video:tag>${video.tag}</video:tag>`,
                    video.rating && `<video:rating>${video.rating}</video:rating>`,
                    video.expiration_date && `<video:expiration_date>${video.expiration_date}</video:expiration_date>`,
                    video.publication_date && `<video:publication_date>${video.publication_date}</video:publication_date>`,
                    video.family_friendly && `<video:family_friendly>${video.family_friendly}</video:family_friendly>`,
                    video.requires_subscription && `<video:requires_subscription>${video.requires_subscription}</video:requires_subscription>`,
                    video.live && `<video:live>${video.live}</video:live>`,
                    video.restriction && `<video:restriction relationship="${video.restriction.relationship}">${video.restriction.content}</video:restriction>`,
                    video.platform && `<video:platform relationship="${video.platform.relationship}">${video.platform.content}</video:platform>`,
                    video.uploader && `<video:uploader${video.uploader.info && ` info="${video.uploader.info}"`}>${video.uploader.content}</video:uploader>`,
                    `</video:video>\n`
                ].filter(Boolean);
                content += videoFields.join('\n');
            }
        }
        if (item.lastModified) {
            const serializedDate = item.lastModified instanceof Date ? item.lastModified.toISOString() : item.lastModified;
            content += `<lastmod>${serializedDate}</lastmod>\n`;
        }
        if (item.changeFrequency) {
            content += `<changefreq>${item.changeFrequency}</changefreq>\n`;
        }
        if (typeof item.priority === 'number') {
            content += `<priority>${item.priority}</priority>\n`;
        }
        content += '</url>\n';
    }
    content += '</urlset>\n';
    return content;
}
function resolveManifest(data) {
    return JSON.stringify(data);
}
function resolveRouteData(data, fileType) {
    if (fileType === 'robots') {
        return resolveRobots(data);
    }
    if (fileType === 'sitemap') {
        return resolveSitemap(data);
    }
    if (fileType === 'manifest') {
        return resolveManifest(data);
    }
    return '';
}

//# sourceMappingURL=resolve-route-data.js.map