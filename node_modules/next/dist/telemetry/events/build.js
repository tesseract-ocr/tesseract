"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
0 && (module.exports = {
    EVENT_BUILD_FEATURE_USAGE: null,
    EVENT_NAME_PACKAGE_USED_IN_GET_SERVER_SIDE_PROPS: null,
    eventBuildCompleted: null,
    eventBuildFeatureUsage: null,
    eventBuildOptimize: null,
    eventLintCheckCompleted: null,
    eventPackageUsedInGetServerSideProps: null,
    eventTypeCheckCompleted: null
});
function _export(target, all) {
    for(var name in all)Object.defineProperty(target, name, {
        enumerable: true,
        get: all[name]
    });
}
_export(exports, {
    EVENT_BUILD_FEATURE_USAGE: function() {
        return EVENT_BUILD_FEATURE_USAGE;
    },
    EVENT_NAME_PACKAGE_USED_IN_GET_SERVER_SIDE_PROPS: function() {
        return EVENT_NAME_PACKAGE_USED_IN_GET_SERVER_SIDE_PROPS;
    },
    eventBuildCompleted: function() {
        return eventBuildCompleted;
    },
    eventBuildFeatureUsage: function() {
        return eventBuildFeatureUsage;
    },
    eventBuildOptimize: function() {
        return eventBuildOptimize;
    },
    eventLintCheckCompleted: function() {
        return eventLintCheckCompleted;
    },
    eventPackageUsedInGetServerSideProps: function() {
        return eventPackageUsedInGetServerSideProps;
    },
    eventTypeCheckCompleted: function() {
        return eventTypeCheckCompleted;
    }
});
const REGEXP_DIRECTORY_DUNDER = /[\\/]__[^\\/]+(?<![\\/]__(?:tests|mocks))__[\\/]/i;
const REGEXP_DIRECTORY_TESTS = /[\\/]__(tests|mocks)__[\\/]/i;
const REGEXP_FILE_TEST = /\.(?:spec|test)\.[^.]+$/i;
const EVENT_TYPE_CHECK_COMPLETED = "NEXT_TYPE_CHECK_COMPLETED";
function eventTypeCheckCompleted(event) {
    return {
        eventName: EVENT_TYPE_CHECK_COMPLETED,
        payload: event
    };
}
const EVENT_LINT_CHECK_COMPLETED = "NEXT_LINT_CHECK_COMPLETED";
function eventLintCheckCompleted(event) {
    return {
        eventName: EVENT_LINT_CHECK_COMPLETED,
        payload: event
    };
}
const EVENT_BUILD_COMPLETED = "NEXT_BUILD_COMPLETED";
function eventBuildCompleted(pagePaths, event) {
    return {
        eventName: EVENT_BUILD_COMPLETED,
        payload: {
            ...event,
            totalPageCount: pagePaths.length,
            hasDunderPages: pagePaths.some((path)=>REGEXP_DIRECTORY_DUNDER.test(path)),
            hasTestPages: pagePaths.some((path)=>REGEXP_DIRECTORY_TESTS.test(path) || REGEXP_FILE_TEST.test(path)),
            totalAppPagesCount: event.totalAppPagesCount
        }
    };
}
const EVENT_BUILD_OPTIMIZED = "NEXT_BUILD_OPTIMIZED";
function eventBuildOptimize(pagePaths, event) {
    return {
        eventName: EVENT_BUILD_OPTIMIZED,
        payload: {
            ...event,
            totalPageCount: pagePaths.length,
            hasDunderPages: pagePaths.some((path)=>REGEXP_DIRECTORY_DUNDER.test(path)),
            hasTestPages: pagePaths.some((path)=>REGEXP_DIRECTORY_TESTS.test(path) || REGEXP_FILE_TEST.test(path)),
            totalAppPagesCount: event.totalAppPagesCount,
            staticAppPagesCount: event.staticAppPagesCount,
            serverAppPagesCount: event.serverAppPagesCount,
            edgeRuntimeAppCount: event.edgeRuntimeAppCount,
            edgeRuntimePagesCount: event.edgeRuntimePagesCount
        }
    };
}
const EVENT_BUILD_FEATURE_USAGE = "NEXT_BUILD_FEATURE_USAGE";
function eventBuildFeatureUsage(usages) {
    return usages.map(({ featureName, invocationCount })=>({
            eventName: EVENT_BUILD_FEATURE_USAGE,
            payload: {
                featureName,
                invocationCount
            }
        }));
}
const EVENT_NAME_PACKAGE_USED_IN_GET_SERVER_SIDE_PROPS = "NEXT_PACKAGE_USED_IN_GET_SERVER_SIDE_PROPS";
function eventPackageUsedInGetServerSideProps(packagesUsedInServerSideProps) {
    return packagesUsedInServerSideProps.map((packageName)=>({
            eventName: EVENT_NAME_PACKAGE_USED_IN_GET_SERVER_SIDE_PROPS,
            payload: {
                package: packageName
            }
        }));
}

//# sourceMappingURL=build.js.map