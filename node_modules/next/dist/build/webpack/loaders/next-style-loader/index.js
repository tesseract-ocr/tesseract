"use strict";
Object.defineProperty(exports, "__esModule", {
    value: true
});
const _path = /*#__PURE__*/ _interop_require_default(require("path"));
const _stringifyrequest = require("../../stringify-request");
function _interop_require_default(obj) {
    return obj && obj.__esModule ? obj : {
        default: obj
    };
}
const loaderApi = ()=>{};
loaderApi.pitch = function loader(request) {
    const loaderSpan = this.currentTraceSpan.traceChild('next-style-loader');
    return loaderSpan.traceFn(()=>{
        const options = this.getOptions();
        const insert = typeof options.insert === 'undefined' ? '"head"' : typeof options.insert === 'string' ? JSON.stringify(options.insert) : options.insert.toString();
        const injectType = options.injectType || 'styleTag';
        const esModule = typeof options.esModule !== 'undefined' ? options.esModule : false;
        delete options.esModule;
        switch(injectType){
            case 'linkTag':
                {
                    const hmrCode = this.hot ? `
if (module.hot) {
  module.hot.accept(
    ${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)},
    function() {
     ${esModule ? 'update(content);' : `content = require(${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)});

           content = content.__esModule ? content.default : content;

           update(content);`}
    }
  );

  module.hot.dispose(function() {
    update();
  });
}` : '';
                    return `${esModule ? `import api from ${(0, _stringifyrequest.stringifyRequest)(this, `!${_path.default.join(__dirname, 'runtime/injectStylesIntoLinkTag.js')}`)};
            import content from ${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)};` : `var api = require(${(0, _stringifyrequest.stringifyRequest)(this, `!${_path.default.join(__dirname, 'runtime/injectStylesIntoLinkTag.js')}`)});
            var content = require(${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)});

            content = content.__esModule ? content.default : content;`}

var options = ${JSON.stringify(options)};

options.insert = ${insert};

var update = api(content, options);

${hmrCode}

${esModule ? 'export default {}' : ''}`;
                }
            case 'lazyStyleTag':
            case 'lazySingletonStyleTag':
                {
                    const isSingleton = injectType === 'lazySingletonStyleTag';
                    const hmrCode = this.hot ? `
if (module.hot) {
  if (!content.locals || module.hot.invalidate) {
    var isEqualLocals = ${require('./runtime/isEqualLocals').toString()};
    console.log({isEqualLocals})
    var oldLocals = content.locals;

    module.hot.accept(
      ${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)},
      function () {
        ${esModule ? `if (!isEqualLocals(oldLocals, content.locals)) {
                module.hot.invalidate();

                return;
              }

              oldLocals = content.locals;

              if (update && refs > 0) {
                update(content);
              }` : `content = require(${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)});

              content = content.__esModule ? content.default : content;

              if (!isEqualLocals(oldLocals, content.locals)) {
                module.hot.invalidate();

                return;
              }

              oldLocals = content.locals;

              if (update && refs > 0) {
                update(content);
              }`}
      }
    )
  }

  module.hot.dispose(function() {
    if (update) {
      update();
    }
  });
}` : '';
                    return `${esModule ? `import api from ${(0, _stringifyrequest.stringifyRequest)(this, `!${_path.default.join(__dirname, 'runtime/injectStylesIntoStyleTag.js')}`)};
            import content from ${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)};` : `var api = require(${(0, _stringifyrequest.stringifyRequest)(this, `!${_path.default.join(__dirname, 'runtime/injectStylesIntoStyleTag.js')}`)});
            var content = require(${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)});

            content = content.__esModule ? content.default : content;

            if (typeof content === 'string') {
              content = [[module.id, content, '']];
            }`}

var refs = 0;
var update;
var options = ${JSON.stringify(options)};

options.insert = ${insert};
options.singleton = ${isSingleton};

var exported = {};

exported.locals = content.locals || {};
exported.use = function() {
  if (!(refs++)) {
    update = api(content, options);
  }

  return exported;
};
exported.unuse = function() {
  if (refs > 0 && !--refs) {
    update();
    update = null;
  }
};

${hmrCode}

${esModule ? 'export default' : 'module.exports ='} exported;`;
                }
            case 'styleTag':
            case 'singletonStyleTag':
            default:
                {
                    const isSingleton = injectType === 'singletonStyleTag';
                    const hmrCode = this.hot ? `
if (module.hot) {
  if (!content.locals || module.hot.invalidate) {
    var isEqualLocals = ${require('./runtime/isEqualLocals').toString()};
    var oldLocals = content.locals;

    module.hot.accept(
      ${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)},
      function () {
        ${esModule ? `if (!isEqualLocals(oldLocals, content.locals)) {
                module.hot.invalidate();

                return;
              }

              oldLocals = content.locals;

              update(content);` : `content = require(${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)});

              content = content.__esModule ? content.default : content;

              if (typeof content === 'string') {
                content = [[module.id, content, '']];
              }

              if (!isEqualLocals(oldLocals, content.locals)) {
                module.hot.invalidate();

                return;
              }

              oldLocals = content.locals;

              update(content);`}
      }
    )
  }

  module.hot.dispose(function() {
    update();
  });
}` : '';
                    return `${esModule ? `import api from ${(0, _stringifyrequest.stringifyRequest)(this, `!${_path.default.join(__dirname, 'runtime/injectStylesIntoStyleTag.js')}`)};
            import content from ${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)};` : `var api = require(${(0, _stringifyrequest.stringifyRequest)(this, `!${_path.default.join(__dirname, 'runtime/injectStylesIntoStyleTag.js')}`)});
            var content = require(${(0, _stringifyrequest.stringifyRequest)(this, `!!${request}`)});

            content = content.__esModule ? content.default : content;

            if (typeof content === 'string') {
              content = [[module.id, content, '']];
            }`}

var options = ${JSON.stringify(options)};

options.insert = ${insert};
options.singleton = ${isSingleton};

var update = api(content, options);

${hmrCode}

${esModule ? 'export default' : 'module.exports ='} content.locals || {};`;
                }
        }
    });
};
module.exports = loaderApi;

//# sourceMappingURL=index.js.map