
var needleInject = 
{
    mSavedFunctions:new Array(),
    /*Returns the first function passed as an argument to the second, allowing you to adjust arguments, run code before and after, and conditionally execute the original function.*/
    wrap: function(obj, func, label) 
    {
       return function() {
            needle.begin(label);
            console.log("MOOOO");
            var v = func.apply(obj, arguments);
            needle.end();
          return v;
        };
    },

    // Call this to inject needle samples into every function of an object in the page
    inject : function()
    {
        var objStack = [];
        for (var i in window) 
        {

            if (typeof window[i] == 'object')
            {
                
                var obj = window[i];
                if(i in this.ignoreObjList) 
                    continue;   //don't inject on us!

                console.log ("-window." + i);
                objStack.push([obj,i]);
            }
            else if(typeof window[i] == 'function')
            {
                 if(i in this.ignoreFcnList) 
                    continue;   //don't inject on us!

                console.log ("-::" + i);

                var safeName = "__NIO__::" + i;
                var lblName = "::" + i;
                var evt = {
                    object:window,
                    property:i,
                    hashName:safeName,
                    fcnPtr:window[i]
                };
                
                this.mSavedFunctions.push(evt);

                window[i] = needleInject.wrap(obj,evt.fcnPtr,lblName);
            }
        }

        while(objStack.length >0)
        {
            var b = objStack.pop();
            var obj = b[0];
            var i = b[1];
            for(var prop in obj)
            {
                if (typeof obj[prop] == 'object')
                {
                    var obj = obj[prop];
                    if(i in this.ignoreObjList) 
                        continue;   //don't inject on us!

                    objStack.push(obj);
                    continue;
                }

                if (typeof obj[prop] != 'function') continue;
                if ( obj[prop] == null) continue;
                console.log ("-" + i + "::" + prop);

                var safeName = "__NIO__" + i + "::" + prop;
                var lblName = i + "::" + prop;
                var evt = {
                    object:obj,
                    property:prop,
                    hashName:safeName,
                    fcnPtr:obj[prop]
                };
                
                this.mSavedFunctions.push(evt);

                obj[prop] = needleInject.wrap(obj,evt.fcnPtr,lblName);

            }
        }
    },

    // ASSUMES that you've already called inject on the page; removes injections back to origional state.
    eject:function()
    {
        for(var i =0; i < this.mSavedFunctions.length; i++)
        {
            var evt = this.mSavedFunctions[i];
            evt.object[evt.property] = evt.fcnPtr;
        }

        this.mSavedFunctions.length = 0;
    }
};

 //here's a list of objects we don't care to hook; either because they aren't relevant, or they may cause issues (like console, needle, date, and performance)
needleInject.ignoreObjList={
//don't touch these unless you want the world to explode
'window':1,
'console':1,
'performance':1,
'Date':1,
'needle':1,
'needleInject':1,
//touch these at your own risk.
'Chrome':1,
'self':1,
'top':1,
'frames':1,
'parent':1,
'navigator':1,
'webkitIndexedDB':1,
'history':1,
'document':1,
'location':1,
'external':1,
'v8Intl':1,
'clientInformation':1,
'applicationCache':1,
'indexedDB':1,
'webkitNotifications':1,
'webkitStorageInfo':1,
'crypto':1,
'styleMedia':1,
"Intl":1,
"chrome":1,
'localStorage':1,
'sessionStorage':1
};

needleInject.ignoreFcnList={
            "$":1,
            "webkitOfflineAudioContext":1,
            
"webkitAudioContext":1,
"webkitSpeechGrammarList":1,
"webkitSpeechGrammar":1,
"webkitSpeechRecognitionEvent":1,
"webkitSpeechRecognitionError":1,
"webkitSpeechRecognition":1,
"WebSocket":1,
"webkitRTCPeerConnection":1,
"webkitMediaStream":1,
"WebKitSourceBufferList":1,
"WebKitSourceBuffer":1,
"WebKitMediaSource":1,
"SharedWorker":1,
"DeviceOrientationEvent":1,
"MediaController":1,
"HTMLSourceElement":1,
"TimeRanges":1,
"MediaError":1,
"HTMLVideoElement":1,
"HTMLMediaElement":1,
"HTMLAudioElement":1,
"Audio":1,
"TrackEvent":1,
"TextTrackList":1,
"TextTrackCueList":1,
"TextTrackCue":1,
"TextTrack":1,
"HTMLTrackElement":1,
"MediaKeyEvent":1,
"MediaKeyError":1,
"HTMLShadowElement":1,
"HTMLContentElement":1,
"WebKitShadowRoot":1,
"OfflineAudioCompletionEvent":1,
"AudioProcessingEvent":1,
"webkitAudioPannerNode":1,
"IDBVersionChangeEvent":1,
"IDBTransaction":1,
"IDBRequest":1,
"IDBOpenDBRequest":1,
"IDBObjectStore":1,
"IDBKeyRange":1,
"IDBIndex":1,
"IDBFactory":1,
"IDBDatabase":1,
"IDBCursorWithValue":1,
"IDBCursor":1,
"webkitIDBTransaction":1,
"webkitIDBRequest":1,
"webkitIDBObjectStore":1,
"webkitIDBKeyRange":1,
"webkitIDBIndex":1,
"webkitIDBFactory":1,
"webkitIDBDatabase":1,
"webkitIDBCursor":1,
"Notification":1,
"CloseEvent":1,
"SQLException":1,
"MediaStreamEvent":1,
"RTCIceCandidate":1,
"RTCSessionDescription":1,
"WebKitMutationObserver":1,
"webkitURL":1,
"URL":1,
"FileReader":1,
"FileError":1,
"FormData":1,
"SVGFilterElement":1,
"SVGFETurbulenceElement":1,
"SVGFETileElement":1,
"SVGFESpotLightElement":1,
"SVGFESpecularLightingElement":1,
"SVGFEPointLightElement":1,
"SVGFEOffsetElement":1,
"SVGFEMorphologyElement":1,
"SVGFEMergeNodeElement":1,
"SVGFEMergeElement":1,
"SVGFEImageElement":1,
"SVGFEGaussianBlurElement":1,
"SVGFEFuncRElement":1,
"SVGFEFuncGElement":1,
"SVGFEFuncBElement":1,
"SVGFEFuncAElement":1,
"SVGFEFloodElement":1,
"SVGFEDropShadowElement":1,
"SVGFEDistantLightElement":1,
"SVGFEDisplacementMapElement":1,
"SVGFEDiffuseLightingElement":1,
"SVGFEConvolveMatrixElement":1,
"SVGFECompositeElement":1,
"SVGFEComponentTransferElement":1,
"SVGFEColorMatrixElement":1,
"SVGFEBlendElement":1,
"SVGComponentTransferFunctionElement":1,
"SVGVKernElement":1,
"SVGMissingGlyphElement":1,
"SVGHKernElement":1,
"SVGGlyphRefElement":1,
"SVGGlyphElement":1,
"SVGFontFaceUriElement":1,
"SVGFontFaceSrcElement":1,
"SVGFontFaceNameElement":1,
"SVGFontFaceFormatElement":1,
"SVGFontFaceElement":1,
"SVGFontElement":1,
"SVGAltGlyphItemElement":1,
"SVGAltGlyphElement":1,
"SVGAltGlyphDefElement":1,
"SVGSetElement":1,
"SVGMPathElement":1,
"SVGAnimateTransformElement":1,
"SVGAnimateMotionElement":1,
"SVGAnimateElement":1,
"SVGAnimateColorElement":1,
"SVGZoomAndPan":1,
"SVGViewSpec":1,
"SVGViewElement":1,
"SVGUseElement":1,
"SVGUnitTypes":1,
"SVGTSpanElement":1,
"SVGTRefElement":1,
"SVGTransformList":1,
"SVGTransform":1,
"SVGTitleElement":1,
"SVGTextPositioningElement":1,
"SVGTextPathElement":1,
"SVGTextElement":1,
"SVGTextContentElement":1,
"SVGSymbolElement":1,
"SVGSwitchElement":1,
"SVGSVGElement":1,
"SVGStyleElement":1,
"SVGStringList":1,
"SVGStopElement":1,
"SVGScriptElement":1,
"SVGRenderingIntent":1,
"SVGRectElement":1,
"SVGRect":1,
"SVGRadialGradientElement":1,
"SVGPreserveAspectRatio":1,
"SVGPolylineElement":1,
"SVGPolygonElement":1,
"SVGPointList":1,
"SVGPoint":1,
"SVGPatternElement":1,
"SVGPathSegMovetoRel":1,
"SVGPathSegMovetoAbs":1,
"SVGPathSegList":1,
"SVGPathSegLinetoVerticalRel":1,
"SVGPathSegLinetoVerticalAbs":1,
"SVGPathSegLinetoRel":1,
"SVGPathSegLinetoHorizontalRel":1,
"SVGPathSegLinetoHorizontalAbs":1,
"SVGPathSegLinetoAbs":1,
"SVGPathSegCurvetoQuadraticSmoothRel":1,
"SVGPathSegCurvetoQuadraticSmoothAbs":1,
"SVGPathSegCurvetoQuadraticRel":1,
"SVGPathSegCurvetoQuadraticAbs":1,
"SVGPathSegCurvetoCubicSmoothRel":1,
"SVGPathSegCurvetoCubicSmoothAbs":1,
"SVGPathSegCurvetoCubicRel":1,
"SVGPathSegCurvetoCubicAbs":1,
"SVGPathSegClosePath":1,
"SVGPathSegArcRel":1,
"SVGPathSegArcAbs":1,
"SVGPathSeg":1,
"SVGPathElement":1,
"SVGPaint":1,
"SVGNumberList":1,
"SVGNumber":1,
"SVGMetadataElement":1,
"SVGMatrix":1,
"SVGMaskElement":1,
"SVGMarkerElement":1,
"SVGLineElement":1,
"SVGLinearGradientElement":1,
"SVGLengthList":1,
"SVGLength":1,
"SVGImageElement":1,
"SVGGradientElement":1,
"SVGGElement":1,
"SVGException":1,
"SVGForeignObjectElement":1,
"SVGEllipseElement":1,
"SVGElementInstanceList":1,
"SVGElementInstance":1,
"SVGElement":1,
"SVGDocument":1,
"SVGDescElement":1,
"SVGDefsElement":1,
"SVGCursorElement":1,
"SVGColor":1,
"SVGClipPathElement":1,
"SVGCircleElement":1,
"SVGAnimatedTransformList":1,
"SVGAnimatedString":1,
"SVGAnimatedRect":1,
"SVGAnimatedPreserveAspectRatio":1,
"SVGAnimatedNumberList":1,
"SVGAnimatedNumber":1,
"SVGAnimatedLengthList":1,
"SVGAnimatedLength":1,
"SVGAnimatedInteger":1,
"SVGAnimatedEnumeration":1,
"SVGAnimatedBoolean":1,
"SVGAnimatedAngle":1,
"SVGAngle":1,
"SVGAElement":1,
"SVGZoomEvent":1,
"XPathException":1,
"XPathResult":1,
"XPathEvaluator":1,
"Storage":1,
"ClientRectList":1,
"ClientRect":1,
"MimeTypeArray":1,
"MimeType":1,
"PluginArray":1,
"Plugin":1,
"MessageChannel":1,
"MessagePort":1,
"XSLTProcessor":1,
"XMLHttpRequestException":1,
"XMLHttpRequestUpload":1,
"XMLHttpRequest":1,
"XMLSerializer":1,
"DOMParser":1,
"XMLDocument":1,
"EventSource":1,
"RangeException":1,
"Range":1,
"NodeFilter":1,
"Blob":1,
"FileList":1,
"File":1,
"Worker":1,
"Clipboard":1,
"WebKitPoint":1,
"WebKitCSSMatrix":1,
"WebKitCSSKeyframesRule":1,
"WebKitCSSKeyframeRule":1,
"EventException":1,
"AutocompleteErrorEvent":1,
"WebGLContextEvent":1,
"SpeechInputEvent":1,
"StorageEvent":1,
"XMLHttpRequestProgressEvent":1,
"WheelEvent":1,
"WebKitTransitionEvent":1,
"WebKitAnimationEvent":1,
/*
-window.eNeedleEventType
-::UIEvent
-::TransitionEvent
-::TextEvent
-::ProgressEvent
-::PageTransitionEvent
-::PopStateEvent
-::OverflowEvent
-::MutationEvent
-::MouseEvent
-::MessageEvent
-::KeyboardEvent
-::HashChangeEvent
-::FocusEvent
-::ErrorEvent
-::CustomEvent
-::CompositionEvent
-::BeforeLoadEvent
-::Event
-::DataView
-::Float32Array
-::Uint32Array
-::Int32Array
-::Uint16Array
-::Int16Array
-::Uint8ClampedArray
-::Uint8Array
-::Int8Array
-::ArrayBufferView
-::ArrayBuffer
-::DOMStringMap
-::WebGLUniformLocation
-::WebGLTexture
-::WebGLShaderPrecisionFormat
-::WebGLShader
-::WebGLRenderingContext
-::WebGLRenderbuffer
-::WebGLProgram
-::WebGLFramebuffer
-::WebGLBuffer
-::WebGLActiveInfo
-::TextMetrics
-::ImageData
-::CanvasRenderingContext2D
-::CanvasGradient
-::CanvasPattern
-::Option
-::Image
-::HTMLUnknownElement
-::HTMLOptionsCollection
-::HTMLFormControlsCollection
-::HTMLAllCollection
-::HTMLCollection
-::HTMLUListElement
-::HTMLTitleElement
-::HTMLTextAreaElement
-::HTMLTemplateElement
-::HTMLTableSectionElement
-::HTMLTableRowElement
-::HTMLTableElement
-::HTMLTableColElement
-::HTMLTableCellElement
-::HTMLTableCaptionElement
-::HTMLStyleElement
-::HTMLSpanElement
-::HTMLSelectElement
-::HTMLScriptElement
-::HTMLQuoteElement
-::HTMLProgressElement
-::HTMLPreElement
-::HTMLParamElement
-::HTMLParagraphElement
-::HTMLOutputElement
-::HTMLOptionElement
-::HTMLOptGroupElement
-::HTMLObjectElement
-::HTMLOListElement
-::HTMLModElement
-::HTMLMeterElement
-::HTMLMetaElement
-::HTMLMenuElement
-::HTMLMarqueeElement
-::HTMLMapElement
-::HTMLLinkElement
-::HTMLLegendElement
-::HTMLLabelElement
-::HTMLLIElement
-::HTMLKeygenElement
-::HTMLInputElement
-::HTMLImageElement
-::HTMLIFrameElement
-::HTMLHtmlElement
-::HTMLHeadingElement
-::HTMLHeadElement
-::HTMLHRElement
-::HTMLFrameSetElement
-::HTMLFrameElement
-::HTMLFormElement
-::HTMLFontElement
-::HTMLFieldSetElement
-::HTMLEmbedElement
-::HTMLDivElement
-::HTMLDirectoryElement
-::HTMLDataListElement
-::HTMLDListElement
-::HTMLCanvasElement
-::HTMLButtonElement
-::HTMLBodyElement
-::HTMLBaseFontElement
-::HTMLBaseElement
-::HTMLBRElement
-::HTMLAreaElement
-::HTMLAppletElement
-::HTMLAnchorElement
-::HTMLElement
-::HTMLDocument
-::Window
-::Selection
-::ProcessingInstruction
-::EntityReference
-::Entity
-::Notation
-::DocumentType
-::CDATASection
-::Comment
-::Text
-::Element
-::Attr
-::CharacterData
-::NamedNodeMap
-::NodeList
-::Node
-::Document
-::DocumentFragment
-::DOMTokenList
-::DOMSettableTokenList
-::DOMImplementation
-::DOMStringList
-::DOMException
-::StyleSheetList
-::RGBColor
-::Rect
-::CSSRuleList
-::Counter
-::MediaList
-::CSSStyleDeclaration
-::CSSStyleRule
-::CSSPageRule
-::CSSMediaRule
-::CSSImportRule
-::CSSHostRule
-::CSSFontFaceRule
-::CSSCharsetRule
-::CSSRule
-::WebKitCSSFilterValue
-::WebKitCSSMixFunctionValue
-::WebKitCSSFilterRule
-::WebKitCSSTransformValue
-::CSSValueList
-::CSSPrimitiveValue
-::CSSValue
-::CSSStyleSheet
-::StyleSheet
-window.opener
-window.frameElement
-window.toolbar
-window.statusbar
-window.scrollbars
-window.personalbar
-window.menubar
-window.locationbar
-window.screen
-::postMessage
-::close
-::blur
-::focus
-window.ondeviceorientation
-window.ontransitionend
-window.onwebkittransitionend
-window.onwebkitanimationstart
-window.onwebkitanimationiteration
-window.onwebkitanimationend
-window.onsearch
-window.onreset
-window.onwaiting
-window.onvolumechange
-window.onunload
-window.ontimeupdate
-window.onsuspend
-window.onsubmit
-window.onstorage
-window.onstalled
-window.onselect
-window.onseeking
-window.onseeked
-window.onscroll
-window.onresize
-window.onratechange
-window.onprogress
-window.onpopstate
-window.onplaying
-window.onplay
-window.onpause
-window.onpageshow
-window.onpagehide
-window.ononline
-window.onoffline
-window.onmousewheel
-window.onmouseup
-window.onmouseover
-window.onmouseout
-window.onmousemove
-window.onmousedown
-window.onmessage
-window.onloadstart
-window.onloadedmetadata
-window.onloadeddata
-window.onload
-window.onkeyup
-window.onkeypress
-window.onkeydown
-window.oninvalid
-window.oninput
-window.onhashchange
-window.onfocus
-window.onerror
-window.onended
-window.onemptied
-window.ondurationchange
-window.ondrop
-window.ondragstart
-window.ondragover
-window.ondragleave
-window.ondragenter
-window.ondragend
-window.ondrag
-window.ondblclick
-window.oncontextmenu
-window.onclick
-window.onchange
-window.oncanplaythrough
-window.oncanplay
-window.onblur
-window.onbeforeunload
-window.onabort
-::getSelection
-::print
-::stop
-::open
-::showModalDialog
-::alert
-::confirm
-::prompt
-::find
-::scrollBy
-::scrollTo
-::scroll
-::moveBy
-::moveTo
-::resizeBy
-::resizeTo
-::matchMedia
-::setTimeout
-::clearTimeout
-::setInterval
-::clearInterval
-::requestAnimationFrame
-::cancelAnimationFrame
-::webkitRequestAnimationFrame
-::webkitCancelAnimationFrame
-::webkitCancelRequestAnimationFrame
-::atob
-::btoa
-::addEventListener
-::removeEventListener
-::captureEvents
-::releaseEvents
-::getComputedStyle
-::getMatchedCSSRules
-::webkitConvertPointFromPageToNode
-::webkitConvertPointFromNodeToPage
-::dispatchEvent
-::webkitRequestFileSystem
-::webkitResolveLocalFileSystemURL
-::openDatabase*/
        };