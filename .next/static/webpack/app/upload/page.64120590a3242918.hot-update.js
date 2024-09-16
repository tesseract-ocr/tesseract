"use strict";
/*
 * ATTENTION: An "eval-source-map" devtool has been used.
 * This devtool is neither made for production nor for readable output files.
 * It uses "eval()" calls to create a separate source file with attached SourceMaps in the browser devtools.
 * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
 * or disable the default devtool with "devtool: false".
 * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
 */
self["webpackHotUpdate_N_E"]("app/upload/page",{

/***/ "(app-pages-browser)/./app/components/ImageUploader.tsx":
/*!******************************************!*\
  !*** ./app/components/ImageUploader.tsx ***!
  \******************************************/
/***/ (function(module, __webpack_exports__, __webpack_require__) {

eval(__webpack_require__.ts("__webpack_require__.r(__webpack_exports__);\n/* harmony import */ var react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__ = __webpack_require__(/*! react/jsx-dev-runtime */ \"(app-pages-browser)/./node_modules/next/dist/compiled/react/jsx-dev-runtime.js\");\n/* harmony import */ var react__WEBPACK_IMPORTED_MODULE_1__ = __webpack_require__(/*! react */ \"(app-pages-browser)/./node_modules/next/dist/compiled/react/index.js\");\n/* harmony import */ var react__WEBPACK_IMPORTED_MODULE_1___default = /*#__PURE__*/__webpack_require__.n(react__WEBPACK_IMPORTED_MODULE_1__);\n/* harmony import */ var axios__WEBPACK_IMPORTED_MODULE_2__ = __webpack_require__(/*! axios */ \"(app-pages-browser)/./node_modules/axios/lib/axios.js\");\n/* __next_internal_client_entry_do_not_use__ default auto */ \nvar _s = $RefreshSig$();\n\n\nconst ImageUploader = ()=>{\n    _s();\n    const [selectedFile, setSelectedFile] = (0,react__WEBPACK_IMPORTED_MODULE_1__.useState)(null);\n    const [result, setResult] = (0,react__WEBPACK_IMPORTED_MODULE_1__.useState)(\"\");\n    const [isLoading, setIsLoading] = (0,react__WEBPACK_IMPORTED_MODULE_1__.useState)(false);\n    const handleFileChange = (event)=>{\n        if (event.target.files && event.target.files.length > 0) {\n            setSelectedFile(event.target.files[0]);\n            handleFileUpload(event.target.files[0]);\n        }\n    };\n    const handleCapturePhoto = (event)=>{\n        if (event.target.files && event.target.files.length > 0) {\n            setSelectedFile(event.target.files[0]);\n            handleFileUpload(event.target.files[0]);\n        }\n    };\n    const handleFileUpload = async (file)=>{\n        setIsLoading(true);\n        const formData = new FormData();\n        formData.append(\"image\", file);\n        try {\n            const response = await axios__WEBPACK_IMPORTED_MODULE_2__[\"default\"].post(\"/api/ocr\", formData, {\n                headers: {\n                    \"Content-Type\": \"multipart/form-data\"\n                }\n            });\n            setResult(response.data.text);\n        } catch (error) {\n            console.error(\"Error uploading image:\", error);\n            alert(\"An error occurred while processing the image\");\n        } finally{\n            setIsLoading(false);\n        }\n    };\n    return /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"div\", {\n        children: [\n            /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"input\", {\n                type: \"file\",\n                accept: \"image/*\",\n                onChange: handleFileChange,\n                style: {\n                    display: \"none\"\n                },\n                id: \"fileInput\"\n            }, void 0, false, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 45,\n                columnNumber: 7\n            }, undefined),\n            /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"label\", {\n                htmlFor: \"fileInput\",\n                className: \"btn btn-primary mr-2\",\n                children: \"Choose File\"\n            }, void 0, false, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 52,\n                columnNumber: 7\n            }, undefined),\n            /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"input\", {\n                type: \"file\",\n                accept: \"image/*\",\n                capture: \"environment\",\n                onChange: handleCapturePhoto,\n                style: {\n                    display: \"none\"\n                },\n                id: \"cameraInput\"\n            }, void 0, false, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 56,\n                columnNumber: 7\n            }, undefined),\n            /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"label\", {\n                htmlFor: \"cameraInput\",\n                className: \"btn btn-secondary\",\n                children: \"Take Photo\"\n            }, void 0, false, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 64,\n                columnNumber: 7\n            }, undefined),\n            /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"input\", {\n                type: \"file\",\n                accept: \"image/*\",\n                capture: \"environment\",\n                onChange: handleCapturePhoto,\n                style: {\n                    display: \"none\"\n                },\n                id: \"cameraInput\"\n            }, void 0, false, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 67,\n                columnNumber: 7\n            }, undefined),\n            isLoading && /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"p\", {\n                children: \"Processing...\"\n            }, void 0, false, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 75,\n                columnNumber: 21\n            }, undefined),\n            result && /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"div\", {\n                children: [\n                    /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"h3\", {\n                        children: \"OCR Result:\"\n                    }, void 0, false, {\n                        fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                        lineNumber: 78,\n                        columnNumber: 11\n                    }, undefined),\n                    /*#__PURE__*/ (0,react_jsx_dev_runtime__WEBPACK_IMPORTED_MODULE_0__.jsxDEV)(\"pre\", {\n                        children: result\n                    }, void 0, false, {\n                        fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                        lineNumber: 79,\n                        columnNumber: 11\n                    }, undefined)\n                ]\n            }, void 0, true, {\n                fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n                lineNumber: 77,\n                columnNumber: 9\n            }, undefined)\n        ]\n    }, void 0, true, {\n        fileName: \"/Users/thun/Desktop/Research Document/tesseract/app/components/ImageUploader.tsx\",\n        lineNumber: 44,\n        columnNumber: 5\n    }, undefined);\n};\n_s(ImageUploader, \"A70gyChoRtFnFZki4eIvYXdOurY=\");\n_c = ImageUploader;\n/* harmony default export */ __webpack_exports__[\"default\"] = (ImageUploader);\nvar _c;\n$RefreshReg$(_c, \"ImageUploader\");\n\n\n;\n    // Wrapped in an IIFE to avoid polluting the global scope\n    ;\n    (function () {\n        var _a, _b;\n        // Legacy CSS implementations will `eval` browser code in a Node.js context\n        // to extract CSS. For backwards compatibility, we need to check we're in a\n        // browser context before continuing.\n        if (typeof self !== 'undefined' &&\n            // AMP / No-JS mode does not inject these helpers:\n            '$RefreshHelpers$' in self) {\n            // @ts-ignore __webpack_module__ is global\n            var currentExports = module.exports;\n            // @ts-ignore __webpack_module__ is global\n            var prevSignature = (_b = (_a = module.hot.data) === null || _a === void 0 ? void 0 : _a.prevSignature) !== null && _b !== void 0 ? _b : null;\n            // This cannot happen in MainTemplate because the exports mismatch between\n            // templating and execution.\n            self.$RefreshHelpers$.registerExportsForReactRefresh(currentExports, module.id);\n            // A module can be accepted automatically based on its exports, e.g. when\n            // it is a Refresh Boundary.\n            if (self.$RefreshHelpers$.isReactRefreshBoundary(currentExports)) {\n                // Save the previous exports signature on update so we can compare the boundary\n                // signatures. We avoid saving exports themselves since it causes memory leaks (https://github.com/vercel/next.js/pull/53797)\n                module.hot.dispose(function (data) {\n                    data.prevSignature =\n                        self.$RefreshHelpers$.getRefreshBoundarySignature(currentExports);\n                });\n                // Unconditionally accept an update to this module, we'll check if it's\n                // still a Refresh Boundary later.\n                // @ts-ignore importMeta is replaced in the loader\n                module.hot.accept();\n                // This field is set when the previous version of this module was a\n                // Refresh Boundary, letting us know we need to check for invalidation or\n                // enqueue an update.\n                if (prevSignature !== null) {\n                    // A boundary can become ineligible if its exports are incompatible\n                    // with the previous exports.\n                    //\n                    // For example, if you add/remove/change exports, we'll want to\n                    // re-execute the importing modules, and force those components to\n                    // re-render. Similarly, if you convert a class component to a\n                    // function, we want to invalidate the boundary.\n                    if (self.$RefreshHelpers$.shouldInvalidateReactRefreshBoundary(prevSignature, self.$RefreshHelpers$.getRefreshBoundarySignature(currentExports))) {\n                        module.hot.invalidate();\n                    }\n                    else {\n                        self.$RefreshHelpers$.scheduleUpdate();\n                    }\n                }\n            }\n            else {\n                // Since we just executed the code for the module, it's possible that the\n                // new exports made it ineligible for being a boundary.\n                // We only care about the case when we were _previously_ a boundary,\n                // because we already accepted this update (accidental side effect).\n                var isNoLongerABoundary = prevSignature !== null;\n                if (isNoLongerABoundary) {\n                    module.hot.invalidate();\n                }\n            }\n        }\n    })();\n//# sourceURL=[module]\n//# sourceMappingURL=data:application/json;charset=utf-8;base64,eyJ2ZXJzaW9uIjozLCJmaWxlIjoiKGFwcC1wYWdlcy1icm93c2VyKS8uL2FwcC9jb21wb25lbnRzL0ltYWdlVXBsb2FkZXIudHN4IiwibWFwcGluZ3MiOiI7Ozs7Ozs7QUFFd0M7QUFDZDtBQUUxQixNQUFNRyxnQkFBMEI7O0lBQzlCLE1BQU0sQ0FBQ0MsY0FBY0MsZ0JBQWdCLEdBQUdKLCtDQUFRQSxDQUFjO0lBQzlELE1BQU0sQ0FBQ0ssUUFBUUMsVUFBVSxHQUFHTiwrQ0FBUUEsQ0FBUztJQUM3QyxNQUFNLENBQUNPLFdBQVdDLGFBQWEsR0FBR1IsK0NBQVFBLENBQVU7SUFFcEQsTUFBTVMsbUJBQW1CLENBQUNDO1FBQ3hCLElBQUlBLE1BQU1DLE1BQU0sQ0FBQ0MsS0FBSyxJQUFJRixNQUFNQyxNQUFNLENBQUNDLEtBQUssQ0FBQ0MsTUFBTSxHQUFHLEdBQUc7WUFDdkRULGdCQUFnQk0sTUFBTUMsTUFBTSxDQUFDQyxLQUFLLENBQUMsRUFBRTtZQUNyQ0UsaUJBQWlCSixNQUFNQyxNQUFNLENBQUNDLEtBQUssQ0FBQyxFQUFFO1FBQ3hDO0lBQ0Y7SUFFQSxNQUFNRyxxQkFBcUIsQ0FBQ0w7UUFDMUIsSUFBSUEsTUFBTUMsTUFBTSxDQUFDQyxLQUFLLElBQUlGLE1BQU1DLE1BQU0sQ0FBQ0MsS0FBSyxDQUFDQyxNQUFNLEdBQUcsR0FBRztZQUN2RFQsZ0JBQWdCTSxNQUFNQyxNQUFNLENBQUNDLEtBQUssQ0FBQyxFQUFFO1lBQ3JDRSxpQkFBaUJKLE1BQU1DLE1BQU0sQ0FBQ0MsS0FBSyxDQUFDLEVBQUU7UUFDeEM7SUFDRjtJQUVBLE1BQU1FLG1CQUFtQixPQUFPRTtRQUM5QlIsYUFBYTtRQUNiLE1BQU1TLFdBQVcsSUFBSUM7UUFDckJELFNBQVNFLE1BQU0sQ0FBQyxTQUFTSDtRQUV6QixJQUFJO1lBQ0YsTUFBTUksV0FBVyxNQUFNbkIsNkNBQUtBLENBQUNvQixJQUFJLENBQUMsWUFBWUosVUFBVTtnQkFDdERLLFNBQVM7b0JBQUUsZ0JBQWdCO2dCQUFzQjtZQUNuRDtZQUNBaEIsVUFBVWMsU0FBU0csSUFBSSxDQUFDQyxJQUFJO1FBQzlCLEVBQUUsT0FBT0MsT0FBTztZQUNkQyxRQUFRRCxLQUFLLENBQUMsMEJBQTBCQTtZQUN4Q0UsTUFBTTtRQUNSLFNBQVU7WUFDUm5CLGFBQWE7UUFDZjtJQUNGO0lBRUEscUJBQ0UsOERBQUNvQjs7MEJBQ0MsOERBQUNDO2dCQUNDQyxNQUFLO2dCQUNMQyxRQUFPO2dCQUNQQyxVQUFVdkI7Z0JBQ1Z3QixPQUFPO29CQUFFQyxTQUFTO2dCQUFPO2dCQUN6QkMsSUFBRzs7Ozs7OzBCQUVMLDhEQUFDQztnQkFBTUMsU0FBUTtnQkFBWUMsV0FBVTswQkFBdUI7Ozs7OzswQkFJNUQsOERBQUNUO2dCQUNDQyxNQUFLO2dCQUNMQyxRQUFPO2dCQUNQUSxTQUFRO2dCQUNSUCxVQUFVakI7Z0JBQ1ZrQixPQUFPO29CQUFFQyxTQUFTO2dCQUFPO2dCQUN6QkMsSUFBRzs7Ozs7OzBCQUVMLDhEQUFDQztnQkFBTUMsU0FBUTtnQkFBY0MsV0FBVTswQkFBb0I7Ozs7OzswQkFHM0QsOERBQUNUO2dCQUNDQyxNQUFLO2dCQUNMQyxRQUFPO2dCQUNQUSxTQUFRO2dCQUNSUCxVQUFVakI7Z0JBQ1ZrQixPQUFPO29CQUFFQyxTQUFTO2dCQUFPO2dCQUN6QkMsSUFBRzs7Ozs7O1lBRUo1QiwyQkFBYSw4REFBQ2lDOzBCQUFFOzs7Ozs7WUFDaEJuQyx3QkFDQyw4REFBQ3VCOztrQ0FDQyw4REFBQ2E7a0NBQUc7Ozs7OztrQ0FDSiw4REFBQ0M7a0NBQUtyQzs7Ozs7Ozs7Ozs7Ozs7Ozs7O0FBS2hCO0dBOUVNSDtLQUFBQTtBQWdGTiwrREFBZUEsYUFBYUEsRUFBQyIsInNvdXJjZXMiOlsid2VicGFjazovL19OX0UvLi9hcHAvY29tcG9uZW50cy9JbWFnZVVwbG9hZGVyLnRzeD9lZDM0Il0sInNvdXJjZXNDb250ZW50IjpbIid1c2UgY2xpZW50JzsgLy8gQWRkIHRoaXMgbGluZSBhdCB0aGUgdG9wIG9mIHRoZSBmaWxlXG5cbmltcG9ydCBSZWFjdCwgeyB1c2VTdGF0ZSB9IGZyb20gJ3JlYWN0JztcbmltcG9ydCBheGlvcyBmcm9tICdheGlvcyc7XG5cbmNvbnN0IEltYWdlVXBsb2FkZXI6IFJlYWN0LkZDID0gKCkgPT4ge1xuICBjb25zdCBbc2VsZWN0ZWRGaWxlLCBzZXRTZWxlY3RlZEZpbGVdID0gdXNlU3RhdGU8RmlsZSB8IG51bGw+KG51bGwpO1xuICBjb25zdCBbcmVzdWx0LCBzZXRSZXN1bHRdID0gdXNlU3RhdGU8c3RyaW5nPignJyk7XG4gIGNvbnN0IFtpc0xvYWRpbmcsIHNldElzTG9hZGluZ10gPSB1c2VTdGF0ZTxib29sZWFuPihmYWxzZSk7XG5cbiAgY29uc3QgaGFuZGxlRmlsZUNoYW5nZSA9IChldmVudDogUmVhY3QuQ2hhbmdlRXZlbnQ8SFRNTElucHV0RWxlbWVudD4pID0+IHtcbiAgICBpZiAoZXZlbnQudGFyZ2V0LmZpbGVzICYmIGV2ZW50LnRhcmdldC5maWxlcy5sZW5ndGggPiAwKSB7XG4gICAgICBzZXRTZWxlY3RlZEZpbGUoZXZlbnQudGFyZ2V0LmZpbGVzWzBdKTtcbiAgICAgIGhhbmRsZUZpbGVVcGxvYWQoZXZlbnQudGFyZ2V0LmZpbGVzWzBdKTtcbiAgICB9XG4gIH07XG5cbiAgY29uc3QgaGFuZGxlQ2FwdHVyZVBob3RvID0gKGV2ZW50OiBSZWFjdC5DaGFuZ2VFdmVudDxIVE1MSW5wdXRFbGVtZW50PikgPT4ge1xuICAgIGlmIChldmVudC50YXJnZXQuZmlsZXMgJiYgZXZlbnQudGFyZ2V0LmZpbGVzLmxlbmd0aCA+IDApIHtcbiAgICAgIHNldFNlbGVjdGVkRmlsZShldmVudC50YXJnZXQuZmlsZXNbMF0pO1xuICAgICAgaGFuZGxlRmlsZVVwbG9hZChldmVudC50YXJnZXQuZmlsZXNbMF0pO1xuICAgIH1cbiAgfTtcblxuICBjb25zdCBoYW5kbGVGaWxlVXBsb2FkID0gYXN5bmMgKGZpbGU6IEZpbGUpID0+IHtcbiAgICBzZXRJc0xvYWRpbmcodHJ1ZSk7XG4gICAgY29uc3QgZm9ybURhdGEgPSBuZXcgRm9ybURhdGEoKTtcbiAgICBmb3JtRGF0YS5hcHBlbmQoJ2ltYWdlJywgZmlsZSk7XG5cbiAgICB0cnkge1xuICAgICAgY29uc3QgcmVzcG9uc2UgPSBhd2FpdCBheGlvcy5wb3N0KCcvYXBpL29jcicsIGZvcm1EYXRhLCB7XG4gICAgICAgIGhlYWRlcnM6IHsgJ0NvbnRlbnQtVHlwZSc6ICdtdWx0aXBhcnQvZm9ybS1kYXRhJyB9LFxuICAgICAgfSk7XG4gICAgICBzZXRSZXN1bHQocmVzcG9uc2UuZGF0YS50ZXh0KTtcbiAgICB9IGNhdGNoIChlcnJvcikge1xuICAgICAgY29uc29sZS5lcnJvcignRXJyb3IgdXBsb2FkaW5nIGltYWdlOicsIGVycm9yKTtcbiAgICAgIGFsZXJ0KCdBbiBlcnJvciBvY2N1cnJlZCB3aGlsZSBwcm9jZXNzaW5nIHRoZSBpbWFnZScpO1xuICAgIH0gZmluYWxseSB7XG4gICAgICBzZXRJc0xvYWRpbmcoZmFsc2UpO1xuICAgIH1cbiAgfTtcblxuICByZXR1cm4gKFxuICAgIDxkaXY+XG4gICAgICA8aW5wdXRcbiAgICAgICAgdHlwZT1cImZpbGVcIlxuICAgICAgICBhY2NlcHQ9XCJpbWFnZS8qXCJcbiAgICAgICAgb25DaGFuZ2U9e2hhbmRsZUZpbGVDaGFuZ2V9XG4gICAgICAgIHN0eWxlPXt7IGRpc3BsYXk6ICdub25lJyB9fVxuICAgICAgICBpZD1cImZpbGVJbnB1dFwiXG4gICAgICAvPlxuICAgICAgPGxhYmVsIGh0bWxGb3I9XCJmaWxlSW5wdXRcIiBjbGFzc05hbWU9XCJidG4gYnRuLXByaW1hcnkgbXItMlwiPlxuICAgICAgICBDaG9vc2UgRmlsZVxuICAgICAgPC9sYWJlbD5cbiAgICAgIFxuICAgICAgPGlucHV0XG4gICAgICAgIHR5cGU9XCJmaWxlXCJcbiAgICAgICAgYWNjZXB0PVwiaW1hZ2UvKlwiXG4gICAgICAgIGNhcHR1cmU9XCJlbnZpcm9ubWVudFwiXG4gICAgICAgIG9uQ2hhbmdlPXtoYW5kbGVDYXB0dXJlUGhvdG99XG4gICAgICAgIHN0eWxlPXt7IGRpc3BsYXk6ICdub25lJyB9fVxuICAgICAgICBpZD1cImNhbWVyYUlucHV0XCJcbiAgICAgIC8+XG4gICAgICA8bGFiZWwgaHRtbEZvcj1cImNhbWVyYUlucHV0XCIgY2xhc3NOYW1lPVwiYnRuIGJ0bi1zZWNvbmRhcnlcIj5cbiAgICAgICAgVGFrZSBQaG90b1xuICAgICAgPC9sYWJlbD5cbiAgICAgIDxpbnB1dFxuICAgICAgICB0eXBlPVwiZmlsZVwiXG4gICAgICAgIGFjY2VwdD1cImltYWdlLypcIlxuICAgICAgICBjYXB0dXJlPVwiZW52aXJvbm1lbnRcIlxuICAgICAgICBvbkNoYW5nZT17aGFuZGxlQ2FwdHVyZVBob3RvfVxuICAgICAgICBzdHlsZT17eyBkaXNwbGF5OiAnbm9uZScgfX1cbiAgICAgICAgaWQ9XCJjYW1lcmFJbnB1dFwiXG4gICAgICAvPlxuICAgICAge2lzTG9hZGluZyAmJiA8cD5Qcm9jZXNzaW5nLi4uPC9wPn1cbiAgICAgIHtyZXN1bHQgJiYgKFxuICAgICAgICA8ZGl2PlxuICAgICAgICAgIDxoMz5PQ1IgUmVzdWx0OjwvaDM+XG4gICAgICAgICAgPHByZT57cmVzdWx0fTwvcHJlPlxuICAgICAgICA8L2Rpdj5cbiAgICAgICl9XG4gICAgPC9kaXY+XG4gICk7XG59O1xuXG5leHBvcnQgZGVmYXVsdCBJbWFnZVVwbG9hZGVyOyJdLCJuYW1lcyI6WyJSZWFjdCIsInVzZVN0YXRlIiwiYXhpb3MiLCJJbWFnZVVwbG9hZGVyIiwic2VsZWN0ZWRGaWxlIiwic2V0U2VsZWN0ZWRGaWxlIiwicmVzdWx0Iiwic2V0UmVzdWx0IiwiaXNMb2FkaW5nIiwic2V0SXNMb2FkaW5nIiwiaGFuZGxlRmlsZUNoYW5nZSIsImV2ZW50IiwidGFyZ2V0IiwiZmlsZXMiLCJsZW5ndGgiLCJoYW5kbGVGaWxlVXBsb2FkIiwiaGFuZGxlQ2FwdHVyZVBob3RvIiwiZmlsZSIsImZvcm1EYXRhIiwiRm9ybURhdGEiLCJhcHBlbmQiLCJyZXNwb25zZSIsInBvc3QiLCJoZWFkZXJzIiwiZGF0YSIsInRleHQiLCJlcnJvciIsImNvbnNvbGUiLCJhbGVydCIsImRpdiIsImlucHV0IiwidHlwZSIsImFjY2VwdCIsIm9uQ2hhbmdlIiwic3R5bGUiLCJkaXNwbGF5IiwiaWQiLCJsYWJlbCIsImh0bWxGb3IiLCJjbGFzc05hbWUiLCJjYXB0dXJlIiwicCIsImgzIiwicHJlIl0sInNvdXJjZVJvb3QiOiIifQ==\n//# sourceURL=webpack-internal:///(app-pages-browser)/./app/components/ImageUploader.tsx\n"));

/***/ })

});