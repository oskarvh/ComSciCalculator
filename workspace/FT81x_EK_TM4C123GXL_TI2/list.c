/*
 * list.c
 *
 *  Created on: 8 sep. 2022
 *      Author: oskar
 */
/* BRIEF: This file contains the linked list functions needed for
 * the computer scientist calculator. it's basically an extended doubly linked list. */
#include "list.h"
// Function to add an entry to the linked list.
// takes in a pointer to the list state,
// the character that should be entered in the list,
// and the index FROM THE LAST ENTRY(!).
// For example, if index = 0, then the entry will be placed at the last
// spot on the list. If index = 1, then the new entry will be placed between
// the last and second to last element.
// If index is larger than the list is long, then the entry will be allocated
// as the first entry of the list.
int addListEntry(listState_t *pListState, char entry, uint8_t index){
    // Get the pointer to the list entry
    listElement_t *pListEntry = pListState->pListEntry;

    // Check if the list is un-initialized
    if(pListEntry == NULL){
        // Allocate the first list entry.
        pListEntry = (listElement_t*)malloc(sizeof(listElement_t));
        if(pListEntry == NULL){
            // Allocation failed, return with failure warning
            return ALLOCATION_FAILED;
        }
        // Enter the character
        pListEntry->currentEntry.currentChar = entry;

        // Since this is the only entry to the list so far, set all pointers to NULL
        pListEntry->pNextElem = NULL;
        pListEntry->pPrevElem = NULL;

        // Set the state variables. The end and start list entries are the same.
        pListState->pListEntry = pListEntry;
        pListState->pListEnd = pListEntry;
        pListState->numEntries += 1;

        // Check if the index was anything else than 0
        if(index > 0){
            return INDEX_TOO_LARGE;
        }
        else{
            return ENTRY_DONE;
        }
    }
    else{
        // A list already exists.
        // Therefore, get the tail end of the list
        listElement_t *pListEnd = pListState->pListEnd;

        // Do a NULL check, and if true there is an error
        if(pListEnd == NULL){
            return LIST_END_ERROR;
        }

        // Iterate through the list index amount of times to find
        // the entry into which this character shall be placed,
        // starting at the end of the list.
        listElement_t *pCurrentElement = pListEnd;
        int i;
        for(i = 0 ; i < index ; i++){
            // Set the current entry to the previous entry
            pCurrentElement = pCurrentElement->pPrevElem;
            if(pCurrentElement == NULL){
                break;
            }
        }

        // Then, insert the new element here.
        // Start by allocating a new element
        listElement_t *pNewElement = (listElement_t*)malloc(sizeof(listElement_t));
        if(pNewElement == NULL){
            // Allocation failed, return with failure warning
            return ALLOCATION_FAILED;
        }
        // Populate the new elements parameters:
        // Enter the character
        pNewElement->currentEntry.currentChar = entry;
        // Squeeze the new element into the list
        if(pCurrentElement == NULL){
            // If the current entry is NULL, that means that
            // we're at the beginning of the list.
            // Therefore, replace the list entry with this entry:

            // Set the next element of the new element to the list entry
            pNewElement->pNextElem = pListState->pListEntry;

            // Set the previous element to NULL to indicate that this
            // is the first entry
            pNewElement->pPrevElem = NULL;

            // Set the list entries previous entry to the new element
            pListState->pListEntry->pPrevElem = pNewElement;

            // Overwrite the first entry in the list state
            pListState->pListEntry = pNewElement;

            // Increase the number of entries:
            pListState->numEntries += 1;
            if(i == index){
                return ENTRY_DONE;
            }
            else{
                return INDEX_TOO_LARGE;
            }
        }
        else {
            // The current entry is not NULL, which means that we're somewhere else in the list.
            // Insert an element in the list OR insert an element at the end of the list.
            // If it's the end of the list, then update the state.
            // Ensure that the list isn't broken when inserting.

            // The current element points to what will be the new element previous element,
            // therefore set the previous element of the new element to the current element
            // (I know, it's confusing..)
            pNewElement->pPrevElem = pCurrentElement;
            // Get the next element in the list, pointed to by the current entry, and
            // set that as the new element next element:
            pNewElement->pNextElem = pCurrentElement->pNextElem;
            // Set the next elements previous element to the new element.
            // If this was the last entry, then set the previous element to the entry,
            // and change the last entry of the state.
            if(pNewElement->pNextElem != NULL){
                ((listElement_t*)(pNewElement->pNextElem))->pPrevElem = pNewElement;
            }
            else{
                pListState->pListEnd = pNewElement;
            }

            // And finally, set the current elements next pointer to this new one:
            pCurrentElement->pNextElem = pNewElement;
            // Increase the number of entries:
            pListState->numEntries += 1;
        }
        return ENTRY_DONE;

    }
}

// Function to remove a list entry
int removeListEntry(listState_t *pListState, uint8_t index){
    // Get the pointer to the list entry
    listElement_t *pListEntry = pListState->pListEntry;

    // Check if the list is un-initialized
    if(pListEntry == NULL){
        // If no entries, then nothing to remove.
        return LIST_EMPTY;
    }
    else{
        // There is an entry, find the pointer to that entry
        // by looping through the list at the index.
        // Get the tail end of the list
        listElement_t *pListEnd = pListState->pListEnd;

        // Do a NULL check, and if true there is an error
        if(pListEnd == NULL){
            return LIST_END_ERROR;
        }

        // Iterate through the list index amount of times to find
        // the entry into which this character shall be placed,
        // starting at the end of the list.
        listElement_t *pCurrentElement = pListEnd;
        int i;
        for(i = 0 ; i < index ; i++){
            // Set the current entry to the previous entry
            pCurrentElement = pCurrentElement->pPrevElem;
            if(pCurrentElement == NULL){
                break;
            }
        }

        if(pCurrentElement == NULL){
            // Points to the start of the buffer, in which case there is nothing to remove.
            return INDEX_TOO_LARGE;
        }

        // pCurrentElement points to the element that should be removed.
        // But before that entry can be removed, check the previous and next entries,
        // and tie them together.
        if(pCurrentElement->pPrevElem != NULL){
            // Not the first entry, therefore change the previous elements next pointer
            // to the current entries next pointer.
            ((listElement_t*)(pCurrentElement->pPrevElem))->pNextElem = pCurrentElement->pNextElem;
        }
        else{
            // Removing the first entry, modify the listState.
            pListState->pListEntry = pCurrentElement->pNextElem;
            // Set the previous element to NULL for the first element in the list.
            pListState->pListEntry->pPrevElem = NULL;
        }
        if(pCurrentElement->pNextElem != NULL){
            // Not the last entry, change the next elements previous element.
            ((listElement_t*)(pCurrentElement->pNextElem))->pPrevElem = pCurrentElement->pPrevElem;
        }
        else{
            // This is the last entry, change the listState and the second to last entry.
            pListState->pListEnd = pCurrentElement->pPrevElem;
            // Change the new list end elements next entry to NULL
            pListState->pListEnd->pNextElem = NULL;
        }
        // Free the entry and return.
        free(pCurrentElement);
        pListState->numEntries -= 1;
        return REMOVE_DONE;
    }
}

// Function to overwrite a list entry
int overwriteListEntry(listState_t *pListState, char entry, uint8_t index){
    // Get the pointer to the list entry
    listElement_t *pListEntry = pListState->pListEntry;

    // Check if the list is empty or we're at the end, just
    // insert at the end. Use the addListEntry function.
    if(pListEntry == NULL || index == 0){
        // If no entries, we should insert instead of trying to overwrite
        return addListEntry(pListState, entry, index);
    }
    else{
        // Index is not 0, find the pointer to that entry
        // by looping through the list at the index.
        // Get the tail end of the list
        listElement_t *pListEnd = pListState->pListEnd;

        // Do a NULL check, and if true there is an error
        if(pListEnd == NULL){
            return LIST_END_ERROR;
        }

        // However, we want to write whatever is after the index,
        // so we need to decrease the index by 1:
        index -=1 ;

        // Iterate through the list index amount of times to find
        // the entry into which this character shall be placed,
        // starting at the end of the list.
        listElement_t *pCurrentElement = pListEnd;
        int i;
        for(i = 0 ; i < index ; i++){
            // Set the current entry to the previous entry
            pCurrentElement = pCurrentElement->pPrevElem;
            if(pCurrentElement == NULL){
                break;
            }
        }

        if(pCurrentElement == NULL){
            // Points to the start of the buffer, in which case there is nothing to remove.
            return INDEX_TOO_LARGE;
        }

        // pCurrentElement points to the element that should be overwritten.
        // However, we only need to overwrite it with the new character!
        pCurrentElement->currentEntry.currentChar = entry;

        return OVERWRITE_DONE;
    }
}

int getCharFromList(listState_t *pListState, char* pCharAtIndex, uint8_t index){
    // Get the tail end of the list
    listElement_t *pListEnd = pListState->pListEnd;

    // Do a NULL check, and if true there is an error
    if(pListEnd == NULL){
        return LIST_END_ERROR;
    }

    // Iterate through the list index amount of times to find
    // the entry into which this character shall be fetched,
    // starting at the end of the list.
    listElement_t *pCurrentElement = pListEnd;
    int i;
    for(i = 0 ; i < index ; i++){
        // Set the current entry to the previous entry
        pCurrentElement = pCurrentElement->pPrevElem;
        if(pCurrentElement == NULL){
            break;
        }
    }

    if(pCurrentElement == NULL){
        // Points to the start of the buffer, in which case there is nothing to remove.
        return INDEX_TOO_LARGE;
    }

    // pCurrentElement points to the element that should be overwritten.
    // However, we only need to overwrite it with the new character!
    *pCharAtIndex = pCurrentElement->currentEntry.currentChar;

    return ENTRY_DONE;
}

