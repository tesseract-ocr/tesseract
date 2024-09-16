"use strict";
/*
 * ATTENTION: An "eval-source-map" devtool has been used.
 * This devtool is neither made for production nor for readable output files.
 * It uses "eval()" calls to create a separate source file with attached SourceMaps in the browser devtools.
 * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
 * or disable the default devtool with "devtool: false".
 * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
 */
(() => {
var exports = {};
exports.id = "app/api/ocr/route";
exports.ids = ["app/api/ocr/route"];
exports.modules = {

/***/ "next/dist/compiled/next-server/app-page.runtime.dev.js":
/*!*************************************************************************!*\
  !*** external "next/dist/compiled/next-server/app-page.runtime.dev.js" ***!
  \*************************************************************************/
/***/ ((module) => {

module.exports = require("next/dist/compiled/next-server/app-page.runtime.dev.js");

/***/ }),

/***/ "next/dist/compiled/next-server/app-route.runtime.dev.js":
/*!**************************************************************************!*\
  !*** external "next/dist/compiled/next-server/app-route.runtime.dev.js" ***!
  \**************************************************************************/
/***/ ((module) => {

module.exports = require("next/dist/compiled/next-server/app-route.runtime.dev.js");

/***/ }),

/***/ "child_process":
/*!********************************!*\
  !*** external "child_process" ***!
  \********************************/
/***/ ((module) => {

module.exports = require("child_process");

/***/ }),

/***/ "fs/promises":
/*!******************************!*\
  !*** external "fs/promises" ***!
  \******************************/
/***/ ((module) => {

module.exports = require("fs/promises");

/***/ }),

/***/ "os":
/*!*********************!*\
  !*** external "os" ***!
  \*********************/
/***/ ((module) => {

module.exports = require("os");

/***/ }),

/***/ "path":
/*!***********************!*\
  !*** external "path" ***!
  \***********************/
/***/ ((module) => {

module.exports = require("path");

/***/ }),

/***/ "util":
/*!***********************!*\
  !*** external "util" ***!
  \***********************/
/***/ ((module) => {

module.exports = require("util");

/***/ }),

/***/ "(rsc)/./node_modules/next/dist/build/webpack/loaders/next-app-loader.js?name=app%2Fapi%2Focr%2Froute&page=%2Fapi%2Focr%2Froute&appPaths=&pagePath=private-next-app-dir%2Fapi%2Focr%2Froute.ts&appDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract%2Fapp&pageExtensions=tsx&pageExtensions=ts&pageExtensions=jsx&pageExtensions=js&rootDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract&isDev=true&tsconfigPath=tsconfig.json&basePath=&assetPrefix=&nextConfigOutput=&preferredRegion=&middlewareConfig=e30%3D!":
/*!***********************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************!*\
  !*** ./node_modules/next/dist/build/webpack/loaders/next-app-loader.js?name=app%2Fapi%2Focr%2Froute&page=%2Fapi%2Focr%2Froute&appPaths=&pagePath=private-next-app-dir%2Fapi%2Focr%2Froute.ts&appDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract%2Fapp&pageExtensions=tsx&pageExtensions=ts&pageExtensions=jsx&pageExtensions=js&rootDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract&isDev=true&tsconfigPath=tsconfig.json&basePath=&assetPrefix=&nextConfigOutput=&preferredRegion=&middlewareConfig=e30%3D! ***!
  \***********************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************/
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

eval("__webpack_require__.r(__webpack_exports__);\n/* harmony export */ __webpack_require__.d(__webpack_exports__, {\n/* harmony export */   originalPathname: () => (/* binding */ originalPathname),\n/* harmony export */   patchFetch: () => (/* binding */ patchFetch),\n/* harmony export */   requestAsyncStorage: () => (/* binding */ requestAsyncStorage),\n/* harmony export */   routeModule: () => (/* binding */ routeModule),\n/* harmony export */   serverHooks: () => (/* binding */ serverHooks),\n/* harmony export */   staticGenerationAsyncStorage: () => (/* binding */ staticGenerationAsyncStorage)\n/* harmony export */ });\n/* harmony import */ var next_dist_server_future_route_modules_app_route_module_compiled__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! next/dist/server/future/route-modules/app-route/module.compiled */ \"(rsc)/./node_modules/next/dist/server/future/route-modules/app-route/module.compiled.js\");\n/* harmony import */ var next_dist_server_future_route_modules_app_route_module_compiled__WEBPACK_IMPORTED_MODULE_0___default = /*#__PURE__*/__webpack_require__.n(next_dist_server_future_route_modules_app_route_module_compiled__WEBPACK_IMPORTED_MODULE_0__);\n/* harmony import */ var next_dist_server_future_route_kind__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! next/dist/server/future/route-kind */ \"(rsc)/./node_modules/next/dist/server/future/route-kind.js\");\n/* harmony import */ var next_dist_server_lib_patch_fetch__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! next/dist/server/lib/patch-fetch */ \"(rsc)/./node_modules/next/dist/server/lib/patch-fetch.js\");\n/* harmony import */ var next_dist_server_lib_patch_fetch__WEBPACK_IMPORTED_MODULE_2___default = /*#__PURE__*/__webpack_require__.n(next_dist_server_lib_patch_fetch__WEBPACK_IMPORTED_MODULE_2__);\n/* harmony import */ var _Users_thun_Desktop_Research_Document_tesseract_app_api_ocr_route_ts__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! ./app/api/ocr/route.ts */ \"(rsc)/./app/api/ocr/route.ts\");\n\n\n\n\n// We inject the nextConfigOutput here so that we can use them in the route\n// module.\nconst nextConfigOutput = \"\"\nconst routeModule = new next_dist_server_future_route_modules_app_route_module_compiled__WEBPACK_IMPORTED_MODULE_0__.AppRouteRouteModule({\n    definition: {\n        kind: next_dist_server_future_route_kind__WEBPACK_IMPORTED_MODULE_1__.RouteKind.APP_ROUTE,\n        page: \"/api/ocr/route\",\n        pathname: \"/api/ocr\",\n        filename: \"route\",\n        bundlePath: \"app/api/ocr/route\"\n    },\n    resolvedPagePath: \"/Users/thun/Desktop/Research Document/tesseract/app/api/ocr/route.ts\",\n    nextConfigOutput,\n    userland: _Users_thun_Desktop_Research_Document_tesseract_app_api_ocr_route_ts__WEBPACK_IMPORTED_MODULE_3__\n});\n// Pull out the exports that we need to expose from the module. This should\n// be eliminated when we've moved the other routes to the new format. These\n// are used to hook into the route.\nconst { requestAsyncStorage, staticGenerationAsyncStorage, serverHooks } = routeModule;\nconst originalPathname = \"/api/ocr/route\";\nfunction patchFetch() {\n    return (0,next_dist_server_lib_patch_fetch__WEBPACK_IMPORTED_MODULE_2__.patchFetch)({\n        serverHooks,\n        staticGenerationAsyncStorage\n    });\n}\n\n\n//# sourceMappingURL=app-route.js.map//# sourceURL=[module]\n//# sourceMappingURL=data:application/json;charset=utf-8;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiKHJzYykvLi9ub2RlX21vZHVsZXMvbmV4dC9kaXN0L2J1aWxkL3dlYnBhY2svbG9hZGVycy9uZXh0LWFwcC1sb2FkZXIuanM/bmFtZT1hcHAlMkZhcGklMkZvY3IlMkZyb3V0ZSZwYWdlPSUyRmFwaSUyRm9jciUyRnJvdXRlJmFwcFBhdGhzPSZwYWdlUGF0aD1wcml2YXRlLW5leHQtYXBwLWRpciUyRmFwaSUyRm9jciUyRnJvdXRlLnRzJmFwcERpcj0lMkZVc2VycyUyRnRodW4lMkZEZXNrdG9wJTJGUmVzZWFyY2glMjBEb2N1bWVudCUyRnRlc3NlcmFjdCUyRmFwcCZwYWdlRXh0ZW5zaW9ucz10c3gmcGFnZUV4dGVuc2lvbnM9dHMmcGFnZUV4dGVuc2lvbnM9anN4JnBhZ2VFeHRlbnNpb25zPWpzJnJvb3REaXI9JTJGVXNlcnMlMkZ0aHVuJTJGRGVza3RvcCUyRlJlc2VhcmNoJTIwRG9jdW1lbnQlMkZ0ZXNzZXJhY3QmaXNEZXY9dHJ1ZSZ0c2NvbmZpZ1BhdGg9dHNjb25maWcuanNvbiZiYXNlUGF0aD0mYXNzZXRQcmVmaXg9Jm5leHRDb25maWdPdXRwdXQ9JnByZWZlcnJlZFJlZ2lvbj0mbWlkZGxld2FyZUNvbmZpZz1lMzAlM0QhIiwibWFwcGluZ3MiOiI7Ozs7Ozs7Ozs7Ozs7OztBQUFzRztBQUN2QztBQUNjO0FBQ29CO0FBQ2pHO0FBQ0E7QUFDQTtBQUNBLHdCQUF3QixnSEFBbUI7QUFDM0M7QUFDQSxjQUFjLHlFQUFTO0FBQ3ZCO0FBQ0E7QUFDQTtBQUNBO0FBQ0EsS0FBSztBQUNMO0FBQ0E7QUFDQSxZQUFZO0FBQ1osQ0FBQztBQUNEO0FBQ0E7QUFDQTtBQUNBLFFBQVEsaUVBQWlFO0FBQ3pFO0FBQ0E7QUFDQSxXQUFXLDRFQUFXO0FBQ3RCO0FBQ0E7QUFDQSxLQUFLO0FBQ0w7QUFDdUg7O0FBRXZIIiwic291cmNlcyI6WyJ3ZWJwYWNrOi8vdGVzc2VyYWN0Lz9jNjgxIl0sInNvdXJjZXNDb250ZW50IjpbImltcG9ydCB7IEFwcFJvdXRlUm91dGVNb2R1bGUgfSBmcm9tIFwibmV4dC9kaXN0L3NlcnZlci9mdXR1cmUvcm91dGUtbW9kdWxlcy9hcHAtcm91dGUvbW9kdWxlLmNvbXBpbGVkXCI7XG5pbXBvcnQgeyBSb3V0ZUtpbmQgfSBmcm9tIFwibmV4dC9kaXN0L3NlcnZlci9mdXR1cmUvcm91dGUta2luZFwiO1xuaW1wb3J0IHsgcGF0Y2hGZXRjaCBhcyBfcGF0Y2hGZXRjaCB9IGZyb20gXCJuZXh0L2Rpc3Qvc2VydmVyL2xpYi9wYXRjaC1mZXRjaFwiO1xuaW1wb3J0ICogYXMgdXNlcmxhbmQgZnJvbSBcIi9Vc2Vycy90aHVuL0Rlc2t0b3AvUmVzZWFyY2ggRG9jdW1lbnQvdGVzc2VyYWN0L2FwcC9hcGkvb2NyL3JvdXRlLnRzXCI7XG4vLyBXZSBpbmplY3QgdGhlIG5leHRDb25maWdPdXRwdXQgaGVyZSBzbyB0aGF0IHdlIGNhbiB1c2UgdGhlbSBpbiB0aGUgcm91dGVcbi8vIG1vZHVsZS5cbmNvbnN0IG5leHRDb25maWdPdXRwdXQgPSBcIlwiXG5jb25zdCByb3V0ZU1vZHVsZSA9IG5ldyBBcHBSb3V0ZVJvdXRlTW9kdWxlKHtcbiAgICBkZWZpbml0aW9uOiB7XG4gICAgICAgIGtpbmQ6IFJvdXRlS2luZC5BUFBfUk9VVEUsXG4gICAgICAgIHBhZ2U6IFwiL2FwaS9vY3Ivcm91dGVcIixcbiAgICAgICAgcGF0aG5hbWU6IFwiL2FwaS9vY3JcIixcbiAgICAgICAgZmlsZW5hbWU6IFwicm91dGVcIixcbiAgICAgICAgYnVuZGxlUGF0aDogXCJhcHAvYXBpL29jci9yb3V0ZVwiXG4gICAgfSxcbiAgICByZXNvbHZlZFBhZ2VQYXRoOiBcIi9Vc2Vycy90aHVuL0Rlc2t0b3AvUmVzZWFyY2ggRG9jdW1lbnQvdGVzc2VyYWN0L2FwcC9hcGkvb2NyL3JvdXRlLnRzXCIsXG4gICAgbmV4dENvbmZpZ091dHB1dCxcbiAgICB1c2VybGFuZFxufSk7XG4vLyBQdWxsIG91dCB0aGUgZXhwb3J0cyB0aGF0IHdlIG5lZWQgdG8gZXhwb3NlIGZyb20gdGhlIG1vZHVsZS4gVGhpcyBzaG91bGRcbi8vIGJlIGVsaW1pbmF0ZWQgd2hlbiB3ZSd2ZSBtb3ZlZCB0aGUgb3RoZXIgcm91dGVzIHRvIHRoZSBuZXcgZm9ybWF0LiBUaGVzZVxuLy8gYXJlIHVzZWQgdG8gaG9vayBpbnRvIHRoZSByb3V0ZS5cbmNvbnN0IHsgcmVxdWVzdEFzeW5jU3RvcmFnZSwgc3RhdGljR2VuZXJhdGlvbkFzeW5jU3RvcmFnZSwgc2VydmVySG9va3MgfSA9IHJvdXRlTW9kdWxlO1xuY29uc3Qgb3JpZ2luYWxQYXRobmFtZSA9IFwiL2FwaS9vY3Ivcm91dGVcIjtcbmZ1bmN0aW9uIHBhdGNoRmV0Y2goKSB7XG4gICAgcmV0dXJuIF9wYXRjaEZldGNoKHtcbiAgICAgICAgc2VydmVySG9va3MsXG4gICAgICAgIHN0YXRpY0dlbmVyYXRpb25Bc3luY1N0b3JhZ2VcbiAgICB9KTtcbn1cbmV4cG9ydCB7IHJvdXRlTW9kdWxlLCByZXF1ZXN0QXN5bmNTdG9yYWdlLCBzdGF0aWNHZW5lcmF0aW9uQXN5bmNTdG9yYWdlLCBzZXJ2ZXJIb29rcywgb3JpZ2luYWxQYXRobmFtZSwgcGF0Y2hGZXRjaCwgIH07XG5cbi8vIyBzb3VyY2VNYXBwaW5nVVJMPWFwcC1yb3V0ZS5qcy5tYXAiXSwibmFtZXMiOltdLCJzb3VyY2VSb290IjoiIn0=\n//# sourceURL=webpack-internal:///(rsc)/./node_modules/next/dist/build/webpack/loaders/next-app-loader.js?name=app%2Fapi%2Focr%2Froute&page=%2Fapi%2Focr%2Froute&appPaths=&pagePath=private-next-app-dir%2Fapi%2Focr%2Froute.ts&appDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract%2Fapp&pageExtensions=tsx&pageExtensions=ts&pageExtensions=jsx&pageExtensions=js&rootDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract&isDev=true&tsconfigPath=tsconfig.json&basePath=&assetPrefix=&nextConfigOutput=&preferredRegion=&middlewareConfig=e30%3D!\n");

/***/ }),

/***/ "(rsc)/./app/api/ocr/route.ts":
/*!******************************!*\
  !*** ./app/api/ocr/route.ts ***!
  \******************************/
/***/ ((__unused_webpack_module, __webpack_exports__, __webpack_require__) => {

eval("__webpack_require__.r(__webpack_exports__);\n/* harmony export */ __webpack_require__.d(__webpack_exports__, {\n/* harmony export */   POST: () => (/* binding */ POST)\n/* harmony export */ });\n/* harmony import */ var next_server__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! next/server */ \"(rsc)/./node_modules/next/dist/api/server.js\");\n/* harmony import */ var fs_promises__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! fs/promises */ \"fs/promises\");\n/* harmony import */ var fs_promises__WEBPACK_IMPORTED_MODULE_1___default = /*#__PURE__*/__webpack_require__.n(fs_promises__WEBPACK_IMPORTED_MODULE_1__);\n/* harmony import */ var child_process__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! child_process */ \"child_process\");\n/* harmony import */ var child_process__WEBPACK_IMPORTED_MODULE_2___default = /*#__PURE__*/__webpack_require__.n(child_process__WEBPACK_IMPORTED_MODULE_2__);\n/* harmony import */ var util__WEBPACK_IMPORTED_MODULE_3__ = __webpack_require__(/*! util */ \"util\");\n/* harmony import */ var util__WEBPACK_IMPORTED_MODULE_3___default = /*#__PURE__*/__webpack_require__.n(util__WEBPACK_IMPORTED_MODULE_3__);\n/* harmony import */ var path__WEBPACK_IMPORTED_MODULE_4__ = __webpack_require__(/*! path */ \"path\");\n/* harmony import */ var path__WEBPACK_IMPORTED_MODULE_4___default = /*#__PURE__*/__webpack_require__.n(path__WEBPACK_IMPORTED_MODULE_4__);\n/* harmony import */ var os__WEBPACK_IMPORTED_MODULE_5__ = __webpack_require__(/*! os */ \"os\");\n/* harmony import */ var os__WEBPACK_IMPORTED_MODULE_5___default = /*#__PURE__*/__webpack_require__.n(os__WEBPACK_IMPORTED_MODULE_5__);\n\n\n\n\n\n\nconst execAsync = (0,util__WEBPACK_IMPORTED_MODULE_3__.promisify)(child_process__WEBPACK_IMPORTED_MODULE_2__.exec);\nasync function POST(request) {\n    try {\n        const formData = await request.formData();\n        const image = formData.get(\"image\");\n        if (!image) {\n            return next_server__WEBPACK_IMPORTED_MODULE_0__.NextResponse.json({\n                error: \"No image uploaded\"\n            }, {\n                status: 400\n            });\n        }\n        // Create a temporary file path\n        const tempDir = os__WEBPACK_IMPORTED_MODULE_5___default().tmpdir();\n        const fileName = `${Date.now()}_${image.name}`;\n        const filePath = path__WEBPACK_IMPORTED_MODULE_4___default().join(tempDir, fileName);\n        // Write the file to the temporary directory\n        const bytes = await image.arrayBuffer();\n        const buffer = Buffer.from(bytes);\n        await (0,fs_promises__WEBPACK_IMPORTED_MODULE_1__.writeFile)(filePath, buffer);\n        // Run Tesseract OCR\n        const { stdout, stderr } = await execAsync(`tesseract \"${filePath}\" stdout -l khm --psm 1`);\n        if (stderr) {\n            console.error(\"Tesseract Error:\", stderr);\n        }\n        // Clean up the temporary file\n        await execAsync(`rm \"${filePath}\"`);\n        return next_server__WEBPACK_IMPORTED_MODULE_0__.NextResponse.json({\n            text: stdout.trim()\n        });\n    } catch (error) {\n        console.error(\"OCR Error:\", error);\n        return next_server__WEBPACK_IMPORTED_MODULE_0__.NextResponse.json({\n            error: \"Error processing image\"\n        }, {\n            status: 500\n        });\n    }\n}\n//# sourceURL=[module]\n//# sourceMappingURL=data:application/json;charset=utf-8;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiKHJzYykvLi9hcHAvYXBpL29jci9yb3V0ZS50cyIsIm1hcHBpbmdzIjoiOzs7Ozs7Ozs7Ozs7Ozs7QUFBd0Q7QUFDaEI7QUFDSDtBQUNKO0FBQ1Q7QUFDSjtBQUVwQixNQUFNTSxZQUFZSCwrQ0FBU0EsQ0FBQ0QsK0NBQUlBO0FBRXpCLGVBQWVLLEtBQUtDLE9BQW9CO0lBQzdDLElBQUk7UUFDRixNQUFNQyxXQUFXLE1BQU1ELFFBQVFDLFFBQVE7UUFDdkMsTUFBTUMsUUFBUUQsU0FBU0UsR0FBRyxDQUFDO1FBRTNCLElBQUksQ0FBQ0QsT0FBTztZQUNWLE9BQU9WLHFEQUFZQSxDQUFDWSxJQUFJLENBQUM7Z0JBQUVDLE9BQU87WUFBb0IsR0FBRztnQkFBRUMsUUFBUTtZQUFJO1FBQ3pFO1FBRUEsK0JBQStCO1FBQy9CLE1BQU1DLFVBQVVWLGdEQUFTO1FBQ3pCLE1BQU1ZLFdBQVcsQ0FBQyxFQUFFQyxLQUFLQyxHQUFHLEdBQUcsQ0FBQyxFQUFFVCxNQUFNVSxJQUFJLENBQUMsQ0FBQztRQUM5QyxNQUFNQyxXQUFXakIsZ0RBQVMsQ0FBQ1csU0FBU0U7UUFFcEMsNENBQTRDO1FBQzVDLE1BQU1NLFFBQVEsTUFBTWIsTUFBTWMsV0FBVztRQUNyQyxNQUFNQyxTQUFTQyxPQUFPQyxJQUFJLENBQUNKO1FBQzNCLE1BQU10QixzREFBU0EsQ0FBQ29CLFVBQVVJO1FBRTFCLG9CQUFvQjtRQUNwQixNQUFNLEVBQUVHLE1BQU0sRUFBRUMsTUFBTSxFQUFFLEdBQUcsTUFBTXZCLFVBQVUsQ0FBQyxXQUFXLEVBQUVlLFNBQVMsdUJBQXVCLENBQUM7UUFFMUYsSUFBSVEsUUFBUTtZQUNWQyxRQUFRakIsS0FBSyxDQUFDLG9CQUFvQmdCO1FBQ3BDO1FBRUEsOEJBQThCO1FBQzlCLE1BQU12QixVQUFVLENBQUMsSUFBSSxFQUFFZSxTQUFTLENBQUMsQ0FBQztRQUVsQyxPQUFPckIscURBQVlBLENBQUNZLElBQUksQ0FBQztZQUFFbUIsTUFBTUgsT0FBT0ksSUFBSTtRQUFHO0lBQ2pELEVBQUUsT0FBT25CLE9BQU87UUFDZGlCLFFBQVFqQixLQUFLLENBQUMsY0FBY0E7UUFDNUIsT0FBT2IscURBQVlBLENBQUNZLElBQUksQ0FBQztZQUFFQyxPQUFPO1FBQXlCLEdBQUc7WUFBRUMsUUFBUTtRQUFJO0lBQzlFO0FBQ0YiLCJzb3VyY2VzIjpbIndlYnBhY2s6Ly90ZXNzZXJhY3QvLi9hcHAvYXBpL29jci9yb3V0ZS50cz80NWQ2Il0sInNvdXJjZXNDb250ZW50IjpbImltcG9ydCB7IE5leHRSZXF1ZXN0LCBOZXh0UmVzcG9uc2UgfSBmcm9tICduZXh0L3NlcnZlcic7XG5pbXBvcnQgeyB3cml0ZUZpbGUgfSBmcm9tICdmcy9wcm9taXNlcyc7XG5pbXBvcnQgeyBleGVjIH0gZnJvbSAnY2hpbGRfcHJvY2Vzcyc7XG5pbXBvcnQgeyBwcm9taXNpZnkgfSBmcm9tICd1dGlsJztcbmltcG9ydCBwYXRoIGZyb20gJ3BhdGgnO1xuaW1wb3J0IG9zIGZyb20gJ29zJztcblxuY29uc3QgZXhlY0FzeW5jID0gcHJvbWlzaWZ5KGV4ZWMpO1xuXG5leHBvcnQgYXN5bmMgZnVuY3Rpb24gUE9TVChyZXF1ZXN0OiBOZXh0UmVxdWVzdCkge1xuICB0cnkge1xuICAgIGNvbnN0IGZvcm1EYXRhID0gYXdhaXQgcmVxdWVzdC5mb3JtRGF0YSgpO1xuICAgIGNvbnN0IGltYWdlID0gZm9ybURhdGEuZ2V0KCdpbWFnZScpIGFzIEZpbGU7XG5cbiAgICBpZiAoIWltYWdlKSB7XG4gICAgICByZXR1cm4gTmV4dFJlc3BvbnNlLmpzb24oeyBlcnJvcjogJ05vIGltYWdlIHVwbG9hZGVkJyB9LCB7IHN0YXR1czogNDAwIH0pO1xuICAgIH1cblxuICAgIC8vIENyZWF0ZSBhIHRlbXBvcmFyeSBmaWxlIHBhdGhcbiAgICBjb25zdCB0ZW1wRGlyID0gb3MudG1wZGlyKCk7XG4gICAgY29uc3QgZmlsZU5hbWUgPSBgJHtEYXRlLm5vdygpfV8ke2ltYWdlLm5hbWV9YDtcbiAgICBjb25zdCBmaWxlUGF0aCA9IHBhdGguam9pbih0ZW1wRGlyLCBmaWxlTmFtZSk7XG5cbiAgICAvLyBXcml0ZSB0aGUgZmlsZSB0byB0aGUgdGVtcG9yYXJ5IGRpcmVjdG9yeVxuICAgIGNvbnN0IGJ5dGVzID0gYXdhaXQgaW1hZ2UuYXJyYXlCdWZmZXIoKTtcbiAgICBjb25zdCBidWZmZXIgPSBCdWZmZXIuZnJvbShieXRlcyk7XG4gICAgYXdhaXQgd3JpdGVGaWxlKGZpbGVQYXRoLCBidWZmZXIpO1xuXG4gICAgLy8gUnVuIFRlc3NlcmFjdCBPQ1JcbiAgICBjb25zdCB7IHN0ZG91dCwgc3RkZXJyIH0gPSBhd2FpdCBleGVjQXN5bmMoYHRlc3NlcmFjdCBcIiR7ZmlsZVBhdGh9XCIgc3Rkb3V0IC1sIGtobSAtLXBzbSAxYCk7XG5cbiAgICBpZiAoc3RkZXJyKSB7XG4gICAgICBjb25zb2xlLmVycm9yKCdUZXNzZXJhY3QgRXJyb3I6Jywgc3RkZXJyKTtcbiAgICB9XG5cbiAgICAvLyBDbGVhbiB1cCB0aGUgdGVtcG9yYXJ5IGZpbGVcbiAgICBhd2FpdCBleGVjQXN5bmMoYHJtIFwiJHtmaWxlUGF0aH1cImApO1xuXG4gICAgcmV0dXJuIE5leHRSZXNwb25zZS5qc29uKHsgdGV4dDogc3Rkb3V0LnRyaW0oKSB9KTtcbiAgfSBjYXRjaCAoZXJyb3IpIHtcbiAgICBjb25zb2xlLmVycm9yKCdPQ1IgRXJyb3I6JywgZXJyb3IpO1xuICAgIHJldHVybiBOZXh0UmVzcG9uc2UuanNvbih7IGVycm9yOiAnRXJyb3IgcHJvY2Vzc2luZyBpbWFnZScgfSwgeyBzdGF0dXM6IDUwMCB9KTtcbiAgfVxufSJdLCJuYW1lcyI6WyJOZXh0UmVzcG9uc2UiLCJ3cml0ZUZpbGUiLCJleGVjIiwicHJvbWlzaWZ5IiwicGF0aCIsIm9zIiwiZXhlY0FzeW5jIiwiUE9TVCIsInJlcXVlc3QiLCJmb3JtRGF0YSIsImltYWdlIiwiZ2V0IiwianNvbiIsImVycm9yIiwic3RhdHVzIiwidGVtcERpciIsInRtcGRpciIsImZpbGVOYW1lIiwiRGF0ZSIsIm5vdyIsIm5hbWUiLCJmaWxlUGF0aCIsImpvaW4iLCJieXRlcyIsImFycmF5QnVmZmVyIiwiYnVmZmVyIiwiQnVmZmVyIiwiZnJvbSIsInN0ZG91dCIsInN0ZGVyciIsImNvbnNvbGUiLCJ0ZXh0IiwidHJpbSJdLCJzb3VyY2VSb290IjoiIn0=\n//# sourceURL=webpack-internal:///(rsc)/./app/api/ocr/route.ts\n");

/***/ })

};
;

// load runtime
var __webpack_require__ = require("../../../webpack-runtime.js");
__webpack_require__.C(exports);
var __webpack_exec__ = (moduleId) => (__webpack_require__(__webpack_require__.s = moduleId))
var __webpack_exports__ = __webpack_require__.X(0, ["vendor-chunks/next"], () => (__webpack_exec__("(rsc)/./node_modules/next/dist/build/webpack/loaders/next-app-loader.js?name=app%2Fapi%2Focr%2Froute&page=%2Fapi%2Focr%2Froute&appPaths=&pagePath=private-next-app-dir%2Fapi%2Focr%2Froute.ts&appDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract%2Fapp&pageExtensions=tsx&pageExtensions=ts&pageExtensions=jsx&pageExtensions=js&rootDir=%2FUsers%2Fthun%2FDesktop%2FResearch%20Document%2Ftesseract&isDev=true&tsconfigPath=tsconfig.json&basePath=&assetPrefix=&nextConfigOutput=&preferredRegion=&middlewareConfig=e30%3D!")));
module.exports = __webpack_exports__;

})();