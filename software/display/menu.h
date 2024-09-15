#ifndef MENU_H_
#define MENU_H_
// Standard library
#include <stdint.h>
#include <stdlib.h>
// Display library
#include "display.h"

#define MAX_MENU_DISPLAY_FUN_STRING 40

typedef struct menuUpdateFunction {
    /**
     * @param interactiveUpdateFun Boolean to tell if the pointer
     * to pUpdateFun is interactive or not.
     * An interactive function means that it hijacks the serial interface and
     * the display, when human input and display (human feedback) is needed.
     * True if it is an interactive function.
     */
    bool interactiveUpdateFun;
    /**
     * @param pUpdateFun Pointer to a function to update this item
     * This is the function that is called when this menu option
     * is selected.
     * If NULL, then no update function exists, and it has to be a sub-menu.
     */
    void *pUpdateFun;
} menuUpdateFunction_t;

/**
 * @brief Type to handle the menu options
 */
typedef struct menuOption {
    /**
     * @param pOptionString Pointer to the string for this option.
     * This is a required option.
     */
    char *pOptionString; /**
                          * @param pSubMenu Pointer to a sub-menu
                          * If this is NULL, then no sub-menu exists.
                          */
    void *pSubMenu;
    /**
     * @param menuUpdateFun Menu update function, can either be interactive or
     * not.
     */
    menuUpdateFunction_t menuUpdateFun;
    /**
     * @param pDisplayFun Pointer to a function that returns the string
     * which shows the current option for this function that is selected.
     * If NULL, then no update function exists, and it has to be a sub-menu.
     * This function takes in a string as an argument, to return to the display
     * function.
     */
    void *pDisplayFun;

} menuOption_t;

/**
 * @brief Struct holding the menu state.
 */
typedef struct menuState {
    /**
     * @param pUpperMenu Pointer to the option list one step above. This is to
     * get back. NULL if this is currently the top level menu. This is a
     * menuState_t pointer.
     */
    void *pUpperMenu;
    /**
     * @param pMenuOptionList Pointer to the menu list
     */
    const menuOption_t *pMenuOptionList;
    /**
     * @param pCurrentMenuOption Pointer to the current option
     */
    menuOption_t *pCurrentMenuOption;
} menuState_t;

// Menu related variables
extern menuState_t bitSizesMenu;
extern menuState_t topMenu;

typedef void non_interactive_menu_function(displayState_t *pDisplayState,
                                           char *pString);
typedef void interactive_menu_function(displayState_t *pDisplayState,
                                       QueueHandle_t *pUartReceiveQueue);

/**
 * @brief Function to get the current font.
 * @param pDisplayState Pointer to the display state.
 * @param pString Pointer to a pre-allocated string that return the output.
 * @return Nothing
 */
void getCurrentFont(displayState_t *pDisplayState, char *pString);
/**
 * @brief Function to get the number of bits that the calculator is configured
 * for.
 * @param pDisplayState Pointer to the display state.
 * @param pString Pointer to a pre-allocated string that return the output.
 * @return Nothing
 */
void getBitSize(displayState_t *pDisplayState, char *pString);
/**
 * @brief Function to get the number of fractional bits for fixed point.
 * @param pDisplayState Pointer to the display state.
 * @param pString Pointer to a pre-allocated string that return the output.
 * @return Nothing
 */
void getFractionalBits(displayState_t *pDisplayState, char *pString);
/**
 * @brief Function to iterate through the current fonts.
 * @param pDisplayState Pointer to the display state.
 * @param pString Pointer to a pre-allocated string that return the output.
 * @return Nothing
 */
void changeFont(displayState_t *pDisplayState, char *pString);
/**
 * @brief Function to go up one menu step.
 * @param pDisplayState Pointer to the display state.
 * @param pString Pointer to a pre-allocated string that return the output.
 * @return Nothing
 */
void goUpOneMenu(displayState_t *pDisplayState, char *pString);
/**
 * @brief Function to exit the menu.
 * @param pDisplayState Pointer to the display state.
 * @param pString Pointer to a pre-allocated string that return the output.
 * @return Nothing
 */
void exitMenu(displayState_t *pDisplayState, char *pString);
/**
 * @brief Interactive menu-function to update the bit width
 * @param pDisplayState Pointer to the display state.
 * @param pUartReceiveQueue Pointer to the UART receive queue
 * @return Nothing
 */
void updateBitWidth(displayState_t *pDisplayState,
                    QueueHandle_t *pUartReceiveQueue);
/**
 * @brief Function to update the display state
 * @param pLocalDisplayState Pointer to the local displayState. This can be read
 * and written to willy nilly
 * @param pGlobalDisplayState Pointer to the global displayState. This needs a
 * semaphore lock to write/read
 * @return Nothing
 */
void updateMenuState(displayState_t *pLocalDisplayState,
                     displayState_t *pGlobalDisplayState);

#endif /* MENU_H_ */