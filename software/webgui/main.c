#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "gui.h"
#include "comscicalc.h"
#include <string.h>

// 4 MB static arena for Clay — avoids any JS-side arena management
#define CLAY_MEMORY_SIZE (6 * 1024 * 1024)
static uint8_t clayMemory[CLAY_MEMORY_SIZE];

// Written every frame by UpdateDrawFrame; JS reads it via GetRenderCommandArrayAddr
static Clay_RenderCommandArray renderCommands;

// Calculator state
static calcCoreState_t calcState;

#define INPUT_TEXT_LEN  512
#define RESULT_TEXT_LEN 128

static char inputText[INPUT_TEXT_LEN]  = "0";
static char hexText[RESULT_TEXT_LEN]   = "0x0";
static char decText[RESULT_TEXT_LEN]   = "0";
static char binText[RESULT_TEXT_LEN]   = "0b0";
static char settingsText[32]           = "DEC";

static const char *const baseNames[3] = { "DEC", "HEX", "BIN" };

static void refreshDisplayStrings(void) {
    int16_t syntaxIssuePos = -1;
    calc_funStatus_t status = calc_printBuffer(
        &calcState, inputText, INPUT_TEXT_LEN, &syntaxIssuePos);
    if (status != calc_funStatus_SUCCESS || inputText[0] == '\0') {
        inputText[0] = '0';
        inputText[1] = '\0';
    }
    convertResult(decText, calcState.result, &calcState.numberFormat, inputBase_DEC);
    convertResult(hexText, calcState.result, &calcState.numberFormat, inputBase_HEX);
    convertResult(binText, calcState.result, &calcState.numberFormat, inputBase_BIN);

    const char *name = baseNames[calcState.numberFormat.inputBase];
    size_t n = strlen(name);
    for (size_t i = 0; i <= n; i++) settingsText[i] = name[i];
}

// Called once from JS after the WASM module is loaded
CLAY_WASM_EXPORT("Init")
void Init(float width, float height) {
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(
        sizeof(clayMemory), clayMemory);
    Clay_Initialize(arena, (Clay_Dimensions){width, height},
                    (Clay_ErrorHandler){0});

    calc_coreInit(&calcState);
    calcState.numberFormat.inputBase    = inputBase_DEC;
    calcState.numberFormat.numBits      = 64;
    calcState.numberFormat.sign         = false;
    calcState.numberFormat.inputFormat  = INPUT_FMT_INT;
    calcState.numberFormat.outputFormat = INPUT_FMT_INT;

    refreshDisplayStrings();
}

// Called from JS on each keypress
CLAY_WASM_EXPORT("AddInput")
void AddInput(int32_t charCode) {
    calc_addInput(&calcState, (char)charCode);
    calc_solver(&calcState);
    refreshDisplayStrings();
}

CLAY_WASM_EXPORT("Backspace")
void Backspace(void) {
    calc_removeInput(&calcState);
    calc_solver(&calcState);
    refreshDisplayStrings();
}

// Tab cycles DEC → HEX → BIN → DEC
CLAY_WASM_EXPORT("CycleBase")
void CycleBase(void) {
    calcState.numberFormat.inputBase =
        (inputBase_t)((calcState.numberFormat.inputBase + 1) % 3);
    calc_updateBase(&calcState);
    calc_solver(&calcState);
    refreshDisplayStrings();
}

// JS calls this once after Init to know where to read render commands
CLAY_WASM_EXPORT("GetRenderCommandArrayAddr")
Clay_RenderCommandArray *GetRenderCommandArrayAddr(void) {
    return &renderCommands;
}

// Called every animation frame from JS
CLAY_WASM_EXPORT("UpdateDrawFrame")
void UpdateDrawFrame(
    float width, float height,
    float mouseWheelX, float mouseWheelY,
    float mousePositionX, float mousePositionY,
    bool isTouchDown, bool isMouseDown,
    bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame,
    float deltaTime)
{
    Clay_SetLayoutDimensions((Clay_Dimensions){ width, height });
    Clay_SetPointerState((Clay_Vector2){ mousePositionX, mousePositionY },
                         isMouseDown || isTouchDown);
    Clay_UpdateScrollContainers(isTouchDown,
                                (Clay_Vector2){ mouseWheelX, mouseWheelY },
                                deltaTime);
    Clay_BeginLayout();
    mainScreen(inputText, hexText, decText, binText, settingsText, 0, 24, width, height);
    renderCommands = Clay_EndLayout();
}

int main(void) {
    return 0;
}
