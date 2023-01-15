/*
 * list.h
 *
 *  Created on: 8 sep. 2022
 *      Author: oskar
 */

#ifndef LIST_H_
#define LIST_H_
#include <xdc/std.h>
// Struct for saving the data and metadata of the current entry
typedef struct calcEntry {
    // The actual character for this entry
    char currentChar;
    // How many parentheses deep this entry is
    // This is generally good to keep track of, and will
    // help with color rendering
    uint8_t depth;
}calcEntry_t;

// Linked list for storing the input.
// We want a doubly linked list, so that characters can be inserted in between
// other characters.
typedef struct listElement{
    // Pointer to next element. If NULL then no next element is defined.
    void *pNextElem;
    // Pointer to previous element. NULL if this is the first element.
    void *pPrevElem;
    // The entry itself.
    calcEntry_t currentEntry;
}listElement_t;

// structure that works to keep track of the state of the list
typedef struct listState {
    listElement_t *pListEntry;
    listElement_t *pListEnd;
    uint8_t numEntries;
}listState_t;

// Different statuses that the linked list helper functions can return
enum listStatus {
    ALLOCATION_FAILED = -1, // new list element allocation failed
    LIST_END_ERROR = -2, // There is an error at the end of the list, e.g NULL pointer
    ENTRY_DONE = 0, // New list element entered at the chosen index
    REMOVE_DONE = 0, // Element removed successfully.
    OVERWRITE_DONE = 0, // Element removed successfully.
    INDEX_TOO_LARGE, // The index input was too large.
    LIST_EMPTY // Trying to remove from an empty list
};

// Functions:
int addListEntry(listState_t *pListState, char entry, uint8_t index);
int removeListEntry(listState_t *pListState, uint8_t index);
int overwriteListEntry(listState_t *pListState, char entry, uint8_t index);
int getCharFromList(listState_t *pListState, char* pCharAtIndex, uint8_t index);
#endif /* LIST_H_ */
