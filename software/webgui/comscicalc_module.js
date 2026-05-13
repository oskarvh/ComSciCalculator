import { WASI } from "https://cdn.jsdelivr.net/npm/@wasmer/wasi@0.10.1/lib/index.esm.js";
const CLAY_RENDER_COMMAND_TYPE_NONE = 0;
const CLAY_RENDER_COMMAND_TYPE_RECTANGLE = 1;
const CLAY_RENDER_COMMAND_TYPE_BORDER = 2;
const CLAY_RENDER_COMMAND_TYPE_TEXT = 3;
const CLAY_RENDER_COMMAND_TYPE_IMAGE = 4;
const CLAY_RENDER_COMMAND_TYPE_SCISSOR_START = 5;
const CLAY_RENDER_COMMAND_TYPE_SCISSOR_END = 6;
const CLAY_RENDER_COMMAND_TYPE_CUSTOM = 7;
const GLOBAL_FONT_SCALING_FACTOR = 0.8;
let renderCommandSize = 0;
let renderCommandArrayAddress = 0;
let memoryDataView;
let textDecoder = new TextDecoder("utf-8");
let previousFrameTime;
let fontsById = [
    'Inconsolata-Regular',
    'Hack-Regular',
    'Monocraft',
    'ComicMono',
    'SourceCodePro-Regular',
    'IBMPlexMono-Regular',
    'VictorMono-Regular',
    'JetBrainsMono-Regular',
];
let elementCache = {};
let imageCache = {};
let dimensionsDefinition = { type: 'struct', members: [
    {name: 'width', type: 'float'},
    {name: 'height', type: 'float'},
]};
let colorDefinition = { type: 'struct', members: [
    {name: 'r', type: 'float' },
    {name: 'g', type: 'float' },
    {name: 'b', type: 'float' },
    {name: 'a', type: 'float' },
]};
let stringDefinition = { type: 'struct', members: [
    {name: 'isStaticallyAllocated', type: 'uint32_t'},
    {name: 'length', type: 'uint32_t' },
    {name: 'chars', type: 'uint32_t' },
]};
let stringSliceDefinition = { type: 'struct', members: [
    {name: 'length', type: 'uint32_t' },
    {name: 'chars', type: 'uint32_t' },
    {name: 'baseChars', type: 'uint32_t' },
]};
let borderWidthDefinition = { type: 'struct', members: [
    {name: 'left', type: 'uint16_t'},
    {name: 'right', type: 'uint16_t'},
    {name: 'top', type: 'uint16_t'},
    {name: 'bottom', type: 'uint16_t'},
    {name: 'betweenChildren', type: 'uint16_t'},
]};
let cornerRadiusDefinition = { type: 'struct', members: [
    {name: 'topLeft', type: 'float'},
    {name: 'topRight', type: 'float'},
    {name: 'bottomLeft', type: 'float'},
    {name: 'bottomRight', type: 'float'},
]};
let textConfigDefinition = { name: 'text', type: 'struct', members: [
    { name: 'userData', type: 'uint32_t' },
    { name: 'textColor', ...colorDefinition },
    { name: 'fontId', type: 'uint16_t' },
    { name: 'fontSize', type: 'uint16_t' },
    { name: 'letterSpacing', type: 'uint16_t' },
    { name: 'lineSpacing', type: 'uint16_t' },
    { name: 'wrapMode', type: 'uint8_t' },
    { name: 'disablePointerEvents', type: 'uint8_t' },
    { name: '_padding', type: 'uint16_t' },
]};
let textRenderDataDefinition = { type: 'struct', members: [
    { name: 'stringContents', ...stringSliceDefinition },
    { name: 'textColor', ...colorDefinition },
    { name: 'fontId', type: 'uint16_t' },
    { name: 'fontSize', type: 'uint16_t' },
    { name: 'letterSpacing', type: 'uint16_t' },
    { name: 'lineHeight', type: 'uint16_t' },
]};
let rectangleRenderDataDefinition = { type: 'struct', members: [
    { name: 'backgroundColor', ...colorDefinition },
    { name: 'cornerRadius', ...cornerRadiusDefinition },
]};
let imageRenderDataDefinition = { type: 'struct', members: [
    { name: 'backgroundColor', ...colorDefinition },
    { name: 'cornerRadius', ...cornerRadiusDefinition },
    { name: 'imageData', type: 'uint32_t' },
]};
let customRenderDataDefinition = { type: 'struct', members: [
    { name: 'backgroundColor', ...colorDefinition },
    { name: 'cornerRadius', ...cornerRadiusDefinition },
    { name: 'customData', type: 'uint32_t' },
]};
let borderRenderDataDefinition = { type: 'struct', members: [
    { name: 'color', ...colorDefinition },
    { name: 'cornerRadius', ...cornerRadiusDefinition },
    { name: 'width', ...borderWidthDefinition },
    { name: 'padding', type: 'uint16_t'}
]};
let clipRenderDataDefinition = { type: 'struct', members: [
    { name: 'horizontal', type: 'bool' },
    { name: 'vertical', type: 'bool' },
]};
let customHTMLDataDefinition = { type: 'struct', members: [
    { name: 'link', ...stringDefinition },
    { name: 'cursorPointer', type: 'uint8_t' },
    { name: 'disablePointerEvents', type: 'uint8_t' },
]};
let renderCommandDefinition = {
    name: 'Clay_RenderCommand',
    type: 'struct',
    members: [
        { name: 'boundingBox', type: 'struct', members: [
            { name: 'x', type: 'float' },
            { name: 'y', type: 'float' },
            { name: 'width', type: 'float' },
            { name: 'height', type: 'float' },
        ]},
        { name: 'renderData', type: 'union', members: [
            { name: 'rectangle', ...rectangleRenderDataDefinition },
            { name: 'text', ...textRenderDataDefinition },
            { name: 'image', ...imageRenderDataDefinition },
            { name: 'custom', ...customRenderDataDefinition },
            { name: 'border', ...borderRenderDataDefinition },
            { name: 'clip', ...clipRenderDataDefinition },
        ]},
        { name: 'userData', type: 'uint32_t'},
        { name: 'id', type: 'uint32_t' },
        { name: 'zIndex', type: 'int16_t' },
        { name: 'commandType', type: 'uint8_t' },
        { name: '_padding', type: 'uint8_t' },
    ]
};

// Function to check if two arrays are different
function MemoryIsDifferent(one, two, length) {
    for (let i = 0; i < length; i++) {
        if (one[i] !== two[i]) {
            return true;
        }
    }
    return false;
}

// Function to set the background and radius of an element
function SetElementBackgroundColorAndRadius(element, cornerRadius, backgroundColor) {
    element.style.backgroundColor = `rgba(${backgroundColor.r.value}, ${backgroundColor.g.value}, ${backgroundColor.b.value}, ${backgroundColor.a.value / 255})`;
    if (cornerRadius.topLeft.value > 0) {
        element.style.borderTopLeftRadius = cornerRadius.topLeft.value + 'px';
    }
    if (cornerRadius.topRight.value > 0) {
        element.style.borderTopRightRadius = cornerRadius.topRight.value + 'px';
    }
    if (cornerRadius.bottomLeft.value > 0) {
        element.style.borderBottomLeftRadius = cornerRadius.bottomLeft.value + 'px';
    }
    if (cornerRadius.bottomRight.value > 0) {
        element.style.borderBottomRightRadius = cornerRadius.bottomRight.value + 'px';
    }
}

function readStructAtAddress(address, definition) {
    switch(definition.type) {
        case 'union':
        case 'struct': {
            let struct = { __size: 0 };
            for (const member of definition.members) {
                let result = readStructAtAddress(address, member);
                struct[member.name] = result;
                if (definition.type === 'struct') {
                    struct.__size += result.__size;
                    address += result.__size;
                } else {
                    struct.__size = Math.max(struct.__size, result.__size);
                }
            }
            return struct;
        }
        case 'float': return { value: memoryDataView.getFloat32(address, true), __size: 4 };
        case 'uint32_t': return { value: memoryDataView.getUint32(address, true), __size: 4 };
        case 'int32_t': return { value: memoryDataView.getUint32(address, true), __size: 4 };
        case 'uint16_t': return { value: memoryDataView.getUint16(address, true), __size: 2 };
        case 'int16_t': return { value: memoryDataView.getInt16(address, true), __size: 2 };
        case 'uint8_t': return { value: memoryDataView.getUint8(address, true), __size: 1 };
        case 'bool': return { value: memoryDataView.getUint8(address, true), __size: 1 };
        default: {
            throw "Unimplemented C data type " + definition.type
        }
    }
}

// HTML render loop
function renderLoopHTML() {
    let length = memoryDataView.getInt32(renderCommandArrayAddress + 4, true);
    let arrayOffset = memoryDataView.getUint32(renderCommandArrayAddress + 8, true);
    let scissorStack = [{ nextAllocation: { x: 0, y: 0 }, element: htmlRoot, nextElementIndex: 0 }];
    let previousId = 0;
    for (let i = 0; i < length; i++, arrayOffset += renderCommandSize) {
        let entireRenderCommandMemory = new Uint8Array(memoryDataView.buffer.slice(arrayOffset, arrayOffset + renderCommandSize));
        let renderCommand = readStructAtAddress(arrayOffset, renderCommandDefinition);
        let parentElement = scissorStack[scissorStack.length - 1];
        let element = null;
        let isMultiConfigElement = previousId === renderCommand.id.value;
        if (!elementCache[renderCommand.id.value]) {
            let elementType = 'div';
            switch (renderCommand.commandType.value & 0xff) {
                case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                    // if (readStructAtAddress(renderCommand.renderData.rectangle.value, rectangleRenderDataDefinition).link.length.value > 0) { TODO reimplement links
                    //     elementType = 'a';
                    // }
                    break;
                }
                case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                    elementType = 'img'; break;
                }
                default: break;
            }
            element = document.createElement(elementType);
            element.id = renderCommand.id.value;
            if (renderCommand.commandType.value === CLAY_RENDER_COMMAND_TYPE_SCISSOR_START) {
                element.style.overflow = 'hidden';
            }
            elementCache[renderCommand.id.value] = {
                exists: true,
                element: element,
                previousMemoryCommand: new Uint8Array(0),
                previousMemoryConfig: new Uint8Array(0),
                previousMemoryText: new Uint8Array(0)
            };
        }

        let elementData = elementCache[renderCommand.id.value];
        element = elementData.element;
        if (!isMultiConfigElement && Array.prototype.indexOf.call(parentElement.element.children, element) !== parentElement.nextElementIndex) {
            if (parentElement.nextElementIndex === 0) {
                parentElement.element.insertAdjacentElement('afterbegin', element);
            } else {
                parentElement.element.childNodes[Math.min(parentElement.nextElementIndex - 1, parentElement.element.childNodes.length - 1)].insertAdjacentElement('afterend', element);
            }
        }

        elementData.exists = true;
        // Don't get me started. Cheaper to compare the render command memory than to update HTML elements
        let dirty = MemoryIsDifferent(elementData.previousMemoryCommand, entireRenderCommandMemory, renderCommandSize) && !isMultiConfigElement;
        if (!isMultiConfigElement) {
            parentElement.nextElementIndex++;
        }

        previousId = renderCommand.id.value;

        elementData.previousMemoryCommand = entireRenderCommandMemory;
        let offsetX = scissorStack.length > 0 ? scissorStack[scissorStack.length - 1].nextAllocation.x : 0;
        let offsetY = scissorStack.length > 0 ? scissorStack[scissorStack.length - 1].nextAllocation.y : 0;
        if (dirty) {
            element.style.transform = `translate(${Math.round(renderCommand.boundingBox.x.value - offsetX)}px, ${Math.round(renderCommand.boundingBox.y.value - offsetY)}px)`
            element.style.width = Math.round(renderCommand.boundingBox.width.value) + 'px';
            element.style.height = Math.round(renderCommand.boundingBox.height.value) + 'px';
        }

        // note: commandType is packed to uint8_t and has 3 garbage bytes of padding
        switch(renderCommand.commandType.value & 0xff) {
            case (CLAY_RENDER_COMMAND_TYPE_NONE): {
                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_RECTANGLE): {
                let config = renderCommand.renderData.rectangle;
                let configMemory = JSON.stringify(config);
                if (configMemory === elementData.previousMemoryConfig) {
                    break;
                }

                SetElementBackgroundColorAndRadius(element, config.cornerRadius, config.backgroundColor);
                if (renderCommand.userData.value !== 0) {
                    let customData = readStructAtAddress(renderCommand.userData.value, customHTMLDataDefinition);

                    let linkContents = customData.link.length.value > 0 ? textDecoder.decode(new Uint8Array(memoryDataView.buffer.slice(customData.link.chars.value, customData.link.chars.value + customData.link.length.value))) : 0;
                    memoryDataView.setUint32(0, renderCommand.id.value, true);
                    if (linkContents.length > 0 && (window.mouseDownThisFrame || window.touchDown) && instance.exports.Clay_PointerOver(0)) {
                        window.location.href = linkContents;
                    }
                    if (linkContents.length > 0) {
                        element.href = linkContents;
                    }

                    if (linkContents.length > 0 || customData.cursorPointer.value) {
                        element.style.pointerEvents = 'all';
                        element.style.cursor = 'pointer';
                    }
                }

                elementData.previousMemoryConfig = configMemory;

                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_BORDER): {
                let config = renderCommand.renderData.border;
                let configMemory = JSON.stringify(config);
                if (configMemory === elementData.previousMemoryConfig) {
                    break;
                }
                let color = config.color;
                elementData.previousMemoryConfig = configMemory;
                if (config.width.left.value > 0) {
                    element.style.borderLeft = `${config.width.left.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
                }
                if (config.width.right.value > 0) {
                    element.style.borderRight = `${config.width.right.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
                }
                if (config.width.top.value > 0) {
                    element.style.borderTop = `${config.width.top.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
                }
                if (config.width.bottom.value > 0) {
                    element.style.borderBottom = `${config.width.bottom.value}px solid rgba(${color.r.value}, ${color.g.value}, ${color.b.value}, ${color.a.value / 255})`
                }
                if (config.cornerRadius.topLeft.value > 0) {
                    element.style.borderTopLeftRadius = config.cornerRadius.topLeft.value + 'px';
                }
                if (config.cornerRadius.topRight.value > 0) {
                    element.style.borderTopRightRadius = config.cornerRadius.topRight.value + 'px';
                }
                if (config.cornerRadius.bottomLeft.value > 0) {
                    element.style.borderBottomLeftRadius = config.cornerRadius.bottomLeft.value + 'px';
                }
                if (config.cornerRadius.bottomRight.value > 0) {
                    element.style.borderBottomRightRadius = config.cornerRadius.bottomRight.value + 'px';
                }
                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_TEXT): {
                let config = renderCommand.renderData.text;
                let customData = readStructAtAddress(renderCommand.userData.value, customHTMLDataDefinition);
                let configMemory = JSON.stringify(config);
                let stringContents = new Uint8Array(memoryDataView.buffer.slice(config.stringContents.chars.value, config.stringContents.chars.value + config.stringContents.length.value));
                if (configMemory !== elementData.previousMemoryConfig) {
                    element.className = 'text';
                    let textColor = config.textColor;
                    let fontSize = Math.round(config.fontSize.value * GLOBAL_FONT_SCALING_FACTOR);
                    element.style.color = `rgba(${textColor.r.value}, ${textColor.g.value}, ${textColor.b.value}, ${textColor.a.value / 255})`;
                    element.style.fontFamily = fontsById[config.fontId.value];
                    element.style.fontSize = fontSize + 'px';
                    element.style.pointerEvents = customData.disablePointerEvents.value ? 'none' : 'all';
                    elementData.previousMemoryConfig = configMemory;
                }
                if (stringContents.length !== elementData.previousMemoryText.length || MemoryIsDifferent(stringContents, elementData.previousMemoryText, stringContents.length)) {
                    element.innerHTML = textDecoder.decode(stringContents);
                }
                elementData.previousMemoryText = stringContents;
                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_SCISSOR_START): {
                scissorStack.push({ nextAllocation: { x: renderCommand.boundingBox.x.value, y: renderCommand.boundingBox.y.value }, element, nextElementIndex: 0 });
                let config = renderCommand.renderData.clip;
                let configMemory = JSON.stringify(config);
                if (configMemory === elementData.previousMemoryConfig) {
                    break;
                }
                if (config.horizontal.value) {
                    element.style.overflowX = 'scroll';
                    element.style.pointerEvents = 'auto';
                }
                if (config.vertical.value) {
                    element.style.overflowY = 'scroll';
                    element.style.pointerEvents = 'auto';
                }
                elementData.previousMemoryConfig = configMemory;
                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_SCISSOR_END): {
                scissorStack.splice(scissorStack.length - 1, 1);
                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_IMAGE): {
                let config = renderCommand.renderData.image;
                let imageURL = readStructAtAddress(config.imageData.value, stringDefinition);
                let srcContents = new Uint8Array(memoryDataView.buffer.slice(imageURL.chars.value, imageURL.chars.value + imageURL.length.value));
                if (srcContents.length !== elementData.previousMemoryText.length || MemoryIsDifferent(srcContents, elementData.previousMemoryText, srcContents.length)) {
                    element.src = textDecoder.decode(srcContents);
                }
                elementData.previousMemoryText = srcContents;
                break;
            }
            case (CLAY_RENDER_COMMAND_TYPE_CUSTOM): break;
            default: {
                console.log("Error: unhandled render command");
            }
        }
    }

    for (const key of Object.keys(elementCache)) {
        if (elementCache[key].exists) {
            elementCache[key].exists = false;
        } else {
            elementCache[key].element.remove();
            delete elementCache[key];
        }
    }
}

// Main renderloop
function renderLoop(currentTime) {
    // Refresh DataView in case WASM memory grew since last frame
    memoryDataView = new DataView(instance.exports.memory.buffer);

    const elapsed = previousFrameTime ? currentTime - previousFrameTime : 0;
    previousFrameTime = currentTime;

    instance.exports.UpdateDrawFrame(
        window.innerWidth, window.innerHeight,
        0, 0,
        window.mousePositionXThisFrame, window.mousePositionYThisFrame,
        window.touchDown ? 1 : 0, window.mouseDown ? 1 : 0,
        window.arrowKeyDownPressedThisFrame ? 1 : 0,
        window.arrowKeyUpPressedThisFrame ? 1 : 0,
        elapsed / 1000
    );

    renderLoopHTML();

    requestAnimationFrame(renderLoop);
    window.mouseDown = false;
    window.arrowKeyUpPressedThisFrame = false;
    window.arrowKeyDownPressedThisFrame = false;
}

// Helper function for fetching the size of structs
function getStructTotalSize(definition) {
    switch(definition.type) {
        case 'union':
        case 'struct': {
            let totalSize = 0;
            for (const member of definition.members) {
                let result = getStructTotalSize(member);
                if (definition.type === 'struct') {
                    totalSize += result;
                } else {
                    totalSize = Math.max(totalSize, result);
                }
            }
            return totalSize;
        }
        case 'float': return 4;
        case 'uint32_t': return 4;
        case 'int32_t': return 4;
        case 'uint16_t': return 2;
        case 'int16_t': return 2;
        case 'uint8_t': return 1;
        case 'bool': return 1;
        default: {
            throw "Unimplemented C data type " + definition.type
        }
    }
}

function getTextDimensions(text, font) {
    // re-use canvas object for better performance
    window.canvasContext.font = font;
    let metrics = window.canvasContext.measureText(text);
    return { width: metrics.width, height: metrics.fontBoundingBoxAscent + metrics.fontBoundingBoxDescent };
}

async function init() {
    await Promise.all(fontsById.map(f => document.fonts.load(`12px "${f}"`)));
    window.htmlRoot = document.body.appendChild(document.createElement('div'));
    window.canvasRoot = document.body.appendChild(document.createElement('canvas'));
    window.canvasContext = window.canvasRoot.getContext("2d");

    window.mousePositionXThisFrame = 0;
    window.mousePositionYThisFrame = 0;
    window.touchDown = false;
    window.mouseDown = false;
    window.arrowKeyDownPressedThisFrame = false;
    window.arrowKeyUpPressedThisFrame = false;

    // Keyboard → calculator input
    document.addEventListener("keydown", (event) => {
        if (!window.instance) return;
        if (event.key === "Backspace") {
            window.instance.exports.Backspace();
            event.preventDefault();
        } else if (event.key === "Tab") {
            window.instance.exports.CycleBase();
            event.preventDefault();
        } else if (event.key === "ArrowDown") {
            window.arrowKeyDownPressedThisFrame = true;
        } else if (event.key === "ArrowUp") {
            window.arrowKeyUpPressedThisFrame = true;
        } else if (event.key.length === 1) {
            window.instance.exports.AddInput(event.key.charCodeAt(0));
        }
    });

    // Minimal WASI shim — logger is disabled at compile time so these are never called,
    // but the module import table still requires them to be present.
    const wasiImports = {
        wasi_snapshot_preview1: {
            fd_write:             () => 0,
            fd_read:              () => 0,
            fd_close:             () => 0,
            fd_seek:              () => 0,
            fd_fdstat_get:        () => 0,
            fd_fdstat_set_flags:  () => 0,
            path_open:            () => 52,
            proc_exit:            () => {},
            args_get:             () => 0,
            args_sizes_get:       (argc, argv_buf_size) => {
                memoryDataView.setUint32(argc, 0, true);
                memoryDataView.setUint32(argv_buf_size, 0, true);
                return 0;
            },
            environ_get:          () => 0,
            environ_sizes_get:    (count, size) => {
                memoryDataView.setUint32(count, 0, true);
                memoryDataView.setUint32(size, 0, true);
                return 0;
            },
            clock_time_get:       () => 0,
            clock_res_get:        () => 0,
            sched_yield:          () => 0,
            random_get:           () => 0,
        }
    };

    const importObject = {
        clay: {
            measureTextFunction: (addressOfDimensions, textToMeasure, addressOfConfig, userData) => {
                let stringLength = memoryDataView.getUint32(textToMeasure, true);
                let pointerToString = memoryDataView.getUint32(textToMeasure + 4, true);
                let textConfig = readStructAtAddress(addressOfConfig, textConfigDefinition);
                let text = textDecoder.decode(memoryDataView.buffer.slice(pointerToString, pointerToString + stringLength));
                let sourceDimensions = getTextDimensions(text, `${Math.round(textConfig.fontSize.value * GLOBAL_FONT_SCALING_FACTOR)}px ${fontsById[textConfig.fontId.value]}`);
                memoryDataView.setFloat32(addressOfDimensions, sourceDimensions.width, true);
                memoryDataView.setFloat32(addressOfDimensions + 4, sourceDimensions.height, true);
            },
            queryScrollOffsetFunction: (addressOfOffset, elementId) => {
                let container = document.getElementById(elementId.toString());
                if (container) {
                    memoryDataView.setFloat32(addressOfOffset, -container.scrollLeft, true);
                    memoryDataView.setFloat32(addressOfOffset + 4, -container.scrollTop, true);
                }
            },
        },
        ...wasiImports,
    };

    const { instance } = await WebAssembly.instantiateStreaming(
        fetch("build/index.wasm"), importObject
    );

    window.instance = instance;
    memoryDataView = new DataView(instance.exports.memory.buffer);

    // Run global C constructors (initialises malloc heap, stack canary, etc.)
    if (instance.exports.__wasm_call_ctors) {
        instance.exports.__wasm_call_ctors();
    }

    // Check Clay's minimum memory requirement before initializing
    const clayMinMemory = instance.exports.Clay_MinMemorySize();
    console.log(`Clay_MinMemorySize = ${clayMinMemory} bytes (${(clayMinMemory/1024/1024).toFixed(2)} MB)`);

    // Initialise Clay + calculator entirely from C
    instance.exports.Init(window.innerWidth, window.innerHeight);

    // Cache the address of the static renderCommands struct
    renderCommandArrayAddress = instance.exports.GetRenderCommandArrayAddr();
    renderCommandSize = getStructTotalSize(renderCommandDefinition);

    requestAnimationFrame(renderLoop);
}

// Initialize
init();