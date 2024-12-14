"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    AppLinksMeta: null,
    OpenGraphMetadata: null,
    TwitterMetadata: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    AppLinksMeta: function() {
        return AppLinksMeta;
    },
    OpenGraphMetadata: function() {
        return OpenGraphMetadata;
    },
    TwitterMetadata: function() {
        return TwitterMetadata;
    }
});
const _meta = require("./meta");
function OpenGraphMetadata({ openGraph }) {
    var _openGraph_title, _openGraph_url, _openGraph_ttl;
    if (!openGraph) {
        return null;
    }
    let typedOpenGraph;
    if ('type' in openGraph) {
        const openGraphType = openGraph.type;
        switch(openGraphType){
            case 'website':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'website'
                    })
                ];
                break;
            case 'article':
                var _openGraph_publishedTime, _openGraph_modifiedTime, _openGraph_expirationTime;
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'article'
                    }),
                    (0, _meta.Meta)({
                        property: 'article:published_time',
                        content: (_openGraph_publishedTime = openGraph.publishedTime) == null ? void 0 : _openGraph_publishedTime.toString()
                    }),
                    (0, _meta.Meta)({
                        property: 'article:modified_time',
                        content: (_openGraph_modifiedTime = openGraph.modifiedTime) == null ? void 0 : _openGraph_modifiedTime.toString()
                    }),
                    (0, _meta.Meta)({
                        property: 'article:expiration_time',
                        content: (_openGraph_expirationTime = openGraph.expirationTime) == null ? void 0 : _openGraph_expirationTime.toString()
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'article:author',
                        contents: openGraph.authors
                    }),
                    (0, _meta.Meta)({
                        property: 'article:section',
                        content: openGraph.section
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'article:tag',
                        contents: openGraph.tags
                    })
                ];
                break;
            case 'book':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'book'
                    }),
                    (0, _meta.Meta)({
                        property: 'book:isbn',
                        content: openGraph.isbn
                    }),
                    (0, _meta.Meta)({
                        property: 'book:release_date',
                        content: openGraph.releaseDate
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'book:author',
                        contents: openGraph.authors
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'book:tag',
                        contents: openGraph.tags
                    })
                ];
                break;
            case 'profile':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'profile'
                    }),
                    (0, _meta.Meta)({
                        property: 'profile:first_name',
                        content: openGraph.firstName
                    }),
                    (0, _meta.Meta)({
                        property: 'profile:last_name',
                        content: openGraph.lastName
                    }),
                    (0, _meta.Meta)({
                        property: 'profile:username',
                        content: openGraph.username
                    }),
                    (0, _meta.Meta)({
                        property: 'profile:gender',
                        content: openGraph.gender
                    })
                ];
                break;
            case 'music.song':
                var _openGraph_duration;
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'music.song'
                    }),
                    (0, _meta.Meta)({
                        property: 'music:duration',
                        content: (_openGraph_duration = openGraph.duration) == null ? void 0 : _openGraph_duration.toString()
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:album',
                        contents: openGraph.albums
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:musician',
                        contents: openGraph.musicians
                    })
                ];
                break;
            case 'music.album':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'music.album'
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:song',
                        contents: openGraph.songs
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:musician',
                        contents: openGraph.musicians
                    }),
                    (0, _meta.Meta)({
                        property: 'music:release_date',
                        content: openGraph.releaseDate
                    })
                ];
                break;
            case 'music.playlist':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'music.playlist'
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:song',
                        contents: openGraph.songs
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:creator',
                        contents: openGraph.creators
                    })
                ];
                break;
            case 'music.radio_station':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'music.radio_station'
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'music:creator',
                        contents: openGraph.creators
                    })
                ];
                break;
            case 'video.movie':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'video.movie'
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:actor',
                        contents: openGraph.actors
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:director',
                        contents: openGraph.directors
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:writer',
                        contents: openGraph.writers
                    }),
                    (0, _meta.Meta)({
                        property: 'video:duration',
                        content: openGraph.duration
                    }),
                    (0, _meta.Meta)({
                        property: 'video:release_date',
                        content: openGraph.releaseDate
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:tag',
                        contents: openGraph.tags
                    })
                ];
                break;
            case 'video.episode':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'video.episode'
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:actor',
                        contents: openGraph.actors
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:director',
                        contents: openGraph.directors
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:writer',
                        contents: openGraph.writers
                    }),
                    (0, _meta.Meta)({
                        property: 'video:duration',
                        content: openGraph.duration
                    }),
                    (0, _meta.Meta)({
                        property: 'video:release_date',
                        content: openGraph.releaseDate
                    }),
                    (0, _meta.MultiMeta)({
                        propertyPrefix: 'video:tag',
                        contents: openGraph.tags
                    }),
                    (0, _meta.Meta)({
                        property: 'video:series',
                        content: openGraph.series
                    })
                ];
                break;
            case 'video.tv_show':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'video.tv_show'
                    })
                ];
                break;
            case 'video.other':
                typedOpenGraph = [
                    (0, _meta.Meta)({
                        property: 'og:type',
                        content: 'video.other'
                    })
                ];
                break;
            default:
                const _exhaustiveCheck = openGraphType;
                throw new Error(`Invalid OpenGraph type: ${_exhaustiveCheck}`);
        }
    }
    return (0, _meta.MetaFilter)([
        (0, _meta.Meta)({
            property: 'og:determiner',
            content: openGraph.determiner
        }),
        (0, _meta.Meta)({
            property: 'og:title',
            content: (_openGraph_title = openGraph.title) == null ? void 0 : _openGraph_title.absolute
        }),
        (0, _meta.Meta)({
            property: 'og:description',
            content: openGraph.description
        }),
        (0, _meta.Meta)({
            property: 'og:url',
            content: (_openGraph_url = openGraph.url) == null ? void 0 : _openGraph_url.toString()
        }),
        (0, _meta.Meta)({
            property: 'og:site_name',
            content: openGraph.siteName
        }),
        (0, _meta.Meta)({
            property: 'og:locale',
            content: openGraph.locale
        }),
        (0, _meta.Meta)({
            property: 'og:country_name',
            content: openGraph.countryName
        }),
        (0, _meta.Meta)({
            property: 'og:ttl',
            content: (_openGraph_ttl = openGraph.ttl) == null ? void 0 : _openGraph_ttl.toString()
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:image',
            contents: openGraph.images
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:video',
            contents: openGraph.videos
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:audio',
            contents: openGraph.audio
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:email',
            contents: openGraph.emails
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:phone_number',
            contents: openGraph.phoneNumbers
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:fax_number',
            contents: openGraph.faxNumbers
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'og:locale:alternate',
            contents: openGraph.alternateLocale
        }),
        ...typedOpenGraph ? typedOpenGraph : []
    ]);
}
function TwitterAppItem({ app, type }) {
    var _app_url_type, _app_url;
    return [
        (0, _meta.Meta)({
            name: `twitter:app:name:${type}`,
            content: app.name
        }),
        (0, _meta.Meta)({
            name: `twitter:app:id:${type}`,
            content: app.id[type]
        }),
        (0, _meta.Meta)({
            name: `twitter:app:url:${type}`,
            content: (_app_url = app.url) == null ? void 0 : (_app_url_type = _app_url[type]) == null ? void 0 : _app_url_type.toString()
        })
    ];
}
function TwitterMetadata({ twitter }) {
    var _twitter_title;
    if (!twitter) return null;
    const { card } = twitter;
    return (0, _meta.MetaFilter)([
        (0, _meta.Meta)({
            name: 'twitter:card',
            content: card
        }),
        (0, _meta.Meta)({
            name: 'twitter:site',
            content: twitter.site
        }),
        (0, _meta.Meta)({
            name: 'twitter:site:id',
            content: twitter.siteId
        }),
        (0, _meta.Meta)({
            name: 'twitter:creator',
            content: twitter.creator
        }),
        (0, _meta.Meta)({
            name: 'twitter:creator:id',
            content: twitter.creatorId
        }),
        (0, _meta.Meta)({
            name: 'twitter:title',
            content: (_twitter_title = twitter.title) == null ? void 0 : _twitter_title.absolute
        }),
        (0, _meta.Meta)({
            name: 'twitter:description',
            content: twitter.description
        }),
        (0, _meta.MultiMeta)({
            namePrefix: 'twitter:image',
            contents: twitter.images
        }),
        ...card === 'player' ? twitter.players.flatMap((player)=>[
                (0, _meta.Meta)({
                    name: 'twitter:player',
                    content: player.playerUrl.toString()
                }),
                (0, _meta.Meta)({
                    name: 'twitter:player:stream',
                    content: player.streamUrl.toString()
                }),
                (0, _meta.Meta)({
                    name: 'twitter:player:width',
                    content: player.width
                }),
                (0, _meta.Meta)({
                    name: 'twitter:player:height',
                    content: player.height
                })
            ]) : [],
        ...card === 'app' ? [
            TwitterAppItem({
                app: twitter.app,
                type: 'iphone'
            }),
            TwitterAppItem({
                app: twitter.app,
                type: 'ipad'
            }),
            TwitterAppItem({
                app: twitter.app,
                type: 'googleplay'
            })
        ] : []
    ]);
}
function AppLinksMeta({ appLinks }) {
    if (!appLinks) return null;
    return (0, _meta.MetaFilter)([
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:ios',
            contents: appLinks.ios
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:iphone',
            contents: appLinks.iphone
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:ipad',
            contents: appLinks.ipad
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:android',
            contents: appLinks.android
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:windows_phone',
            contents: appLinks.windows_phone
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:windows',
            contents: appLinks.windows
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:windows_universal',
            contents: appLinks.windows_universal
        }),
        (0, _meta.MultiMeta)({
            propertyPrefix: 'al:web',
            contents: appLinks.web
        })
    ]);
}

//# sourceMappingURL=opengraph.js.map