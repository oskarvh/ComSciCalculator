#define CLAY_IMPLEMENTATION
#include "clay.h"

typedef struct {
    void* memory;
    uintptr_t offset;
} Arena;

Arena frameArena = {};

double windowWidth = 1024, windowHeight = 768;

uint32_t ACTIVE_RENDERER_INDEX = 0;

CLAY_WASM_EXPORT("SetScratchMemory") void SetScratchMemory(void * memory) {
    frameArena.memory = memory;
}

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(float width, float height, float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY, bool isTouchDown, bool isMouseDown, bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame, bool dKeyPressedThisFrame, float deltaTime) {
    frameArena.offset = 0;
    windowWidth = width;
    windowHeight = height;
    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
    Clay_ScrollContainerData scrollContainerData = Clay_GetScrollContainerData(Clay_GetElementId(CLAY_STRING("OuterScrollContainer")));
    Clay_LayoutElementHashMapItem *perfPage = Clay__GetHashMapItem(Clay_GetElementId(CLAY_STRING("PerformanceOuter")).id);
    // NaN propagation can cause pain here
    float perfPageYOffset = perfPage->boundingBox.y + scrollContainerData.scrollPosition->y;
    if (deltaTime == deltaTime && (ACTIVE_RENDERER_INDEX == 1 || (perfPageYOffset < height && perfPageYOffset + perfPage->boundingBox.height > 0))) {
        animationLerpValue += deltaTime;
        if (animationLerpValue > 1) {
            animationLerpValue -= 2;
        }
    }

    // if (dKeyPressedThisFrame) {
    //     debugModeEnabled = !debugModeEnabled;
    //     Clay_SetDebugModeEnabled(debugModeEnabled);
    // }
    Clay_SetCullingEnabled(ACTIVE_RENDERER_INDEX == 1);
    Clay_SetExternalScrollHandlingEnabled(ACTIVE_RENDERER_INDEX == 0);

    Clay__debugViewHighlightColor = (Clay_Color) {105,210,231, 120};

    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);

    // if (!isMouseDown) {
    //     scrollbarData.mouseDown = false;
    // }

    if (isMouseDown && !scrollbarData.mouseDown && Clay_PointerOver(Clay_GetElementId(CLAY_STRING("ScrollBar")))) {
        scrollbarData.clickOrigin = (Clay_Vector2) { mousePositionX, mousePositionY };
        scrollbarData.positionOrigin = *scrollContainerData.scrollPosition;
        scrollbarData.mouseDown = true;
    } else if (scrollbarData.mouseDown) {
        if (scrollContainerData.contentDimensions.height > 0) {
            Clay_Vector2 ratio = (Clay_Vector2) {
                scrollContainerData.contentDimensions.width / scrollContainerData.scrollContainerDimensions.width,
                scrollContainerData.contentDimensions.height / scrollContainerData.scrollContainerDimensions.height,
            };
            if (scrollContainerData.config.vertical) {
                scrollContainerData.scrollPosition->y = scrollbarData.positionOrigin.y + (scrollbarData.clickOrigin.y - mousePositionY) * ratio.y;
            }
            if (scrollContainerData.config.horizontal) {
                scrollContainerData.scrollPosition->x = scrollbarData.positionOrigin.x + (scrollbarData.clickOrigin.x - mousePositionX) * ratio.x;
            }
        }
    }

    if (arrowKeyDownPressedThisFrame) {
        if (scrollContainerData.contentDimensions.height > 0) {
            scrollContainerData.scrollPosition->y = scrollContainerData.scrollPosition->y - 50;
        }
    } else if (arrowKeyUpPressedThisFrame) {
        if (scrollContainerData.contentDimensions.height > 0) {
            scrollContainerData.scrollPosition->y = scrollContainerData.scrollPosition->y + 50;
        }
    }

    Clay_UpdateScrollContainers(isTouchDown, (Clay_Vector2) {mouseWheelX, mouseWheelY}, deltaTime);
    bool isMobileScreen = windowWidth < 750;
    if (debugModeEnabled) {
        isMobileScreen = windowWidth < 950;
    }
    return CreateLayout(isMobileScreen, animationLerpValue < 0 ? (animationLerpValue + 1) : (1 - animationLerpValue));
    //----------------------------------------------------------------------------------
}

int main(void)
{
    puts("Hello");
    return 0;
}