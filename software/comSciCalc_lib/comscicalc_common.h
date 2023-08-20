/*
MIT License

Copyright (c) 2023 Oskar von Heideken

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef COMSCICALC_COMMON_H
#define COMSCICALC_COMMON_H

#include <stdbool.h>
#include <stdint.h>

//! Maximum length of the operator string
#define OPERATOR_STRING_MAX_LEN 10

/**
 * @defgroup entryFlagDefs Defines for entry flags
 * @{
 */
#define INPUT_TYPE_EMPTY 0
#define INPUT_TYPE_NUMBER 1
#define INPUT_TYPE_OPERATOR 2
#define INPUT_TYPE_DECIMAL_POINT 3
#define DEPTH_CHANGE_KEEP 0
#define DEPTH_CHANGE_INCREASE 1
#define DEPTH_CHANGE_DECREASE 2
#define DEPTH_CHANGE_RESERVED 3
#define SUBRESULT_TYPE_CHAR 0
#define SUBRESULT_TYPE_INT 1
#define INPUT_FMT_INT 0
#define INPUT_FMT_FIXED 1
#define INPUT_FMT_FLOAT 2
#define INPUT_FMT_RESERVED 3

/**@}*/
/* -------------------------------------------
 * ----------------- MACROS ------------------
 * -------------------------------------------*/
/**
 * @defgroup entryFlagMacros Macros for entry flags
 * @{
 */
#define CONSTRUCT_TYPEFLAG(sign, inputFormat, subResType, depthFlag,           \
                           inputType)                                          \
    (sign << 7 | inputFormat << 5 | subResType << 4 | depthFlag << 2 |         \
     inputType)
#define GET_DEPTH_FLAG(typeFlag) ((typeFlag >> 2) & 0x3)
#define GET_INPUT_TYPE(typeFlag) ((typeFlag)&0x3)
#define GET_SUBRESULT_TYPE(typeFlag) ((typeFlag >> 4) & 0x1)
#define GET_FMT_TYPE(typeFlag) ((typeFlag >> 5) & 0x3)

/**@}*/
/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
//! Typedef to hold the typeFlag
typedef uint8_t typeFlag_t;

//! Typedef to hold the input format
typedef uint8_t inputFormat_t;

/**
 * @defgroup subresTypedefs Typedefs for sub results.
 * @note These define the type for which the result and
 *   subresults are calculated and displayed with.
 * @warning These shall be removed and replaced with numberFormat
 * @{
 */
typedef int64_t SUBRESULT_INT;
/**@}*/

//! Typedef for the input base (dec, hex, bin, none)
typedef uint8_t inputBase_t;
/**
 * @brief Struct for holding input data and sub result data.
 *
 * This struct holds the data entered by the user, along with
 * storing sub results used while solving the expression.
 */
typedef struct inputType {
    /**
     * @param c Input character.
     */
    char c;
    /**
     * @param typeFlag Flag holding metadata of the entry
     *
     * Call me old school but I try to avoid bitfields.
     * bit 0-1: 0 = empty, 1 = number, 3 = operator, 3 = custom function.
     * bit 2-3: 0 = keep depth, 1 = increase depth, 2 = decrease depth, 3 =
     * reserved.
     * bit 4: 0 = char input, 1 = subresult.
     * Bit 5-6: Input is: 0 = int, 1 = floating point
     * 2 = fixed point, 3 = reserved
     * bit 7: 1 = signed, 0 = unsigned.
     */
    typeFlag_t typeFlag;

    //! Partial restult. Only used for when solving.
    SUBRESULT_INT subresult;
} inputType_t;

/**
 * @brief Struct for holding operator entry.
 *
 * This struct defines the data and metadata needed for the
 * operator entry list.
 */
typedef struct operatorEntry {
    /**
     * @param inputChar Char to which this enties function is associated with.
     *
     * This is what the user enters to get the functionality of this entry.
     */
    char inputChar;
    /**
     * @param opString String associated with this function.
     *
     * The user only enters one char to get this function,
     * but there might be a case where another string should be displayed
     * to the user, for readibility reasons.
     */
    char opString[OPERATOR_STRING_MAX_LEN];
    /**
     * @param solvPrio Priority of solving.
     *
     * The priority of which to solve this. E.g. * has higher prio
     * than + in 123+456*789. Therefore * has higher priority.
     * @note Priority is ascending, so 0 has the highest priority,
     *   255 has the lowest.
     */
    uint8_t solvPrio;
    /**
     * @param bIncDepth Flag to increase depth
     *
     * True if this function increases the depth, i.e.
     * a bracket is needed after the function. E.g. sin is
     * a depth increasing, since sin had to be followed by a bracket:
     * sin(x).
     */
    bool bIncDepth;
    /**
     * @param pDoc Pointer to documentation string.
     *
     * Points to a \0-terminated string which describes the function.
     * This will be displayed in the help section in the calculator.
     */
    char *pDoc;
    /**
     * @param pFun Pointer to the calculator function
     *
     * Points to the function associated with this input.
     * @note This will be typecasted to the correct format.
     */
    void *pFun;
    // Number of arguments.
    // -1: variable arguments, give input as pointers.
    // 0 : reserved(use for variable maybe?)
    // >0: number of arguments accepted.
    // NOTE: for non-depth increasing operators,
    // this must be 2. All others are ignored.
    /**
     * @param numArgs Number of arguments for the calculation function.
     *
     * -1 is variable arguments.
     * 0 is reserved.
     * >0 means fixed set of arguments.
     * @note for non-depth increasing operators,
     *   this must be 2. All others are ignored.
     */
    int8_t numArgs;
} operatorEntry_t;

/**
 * @brief Struct holding the number format for the current entry
 *
 * @note Some of these might change throughout an expression,
 * such as the base such as  mixing binary and decimal numbers in
 * an expression, e.g., 0b101+0xff.
 * Some might not however, such as the number of bytes,
 * fixed point decimal place and so on.
 */
typedef struct numberFormat {
    /**
     * @param numBits Number of bits used in calculations
     * @note Maximum is 64 bits, and the output will be
     * truncated to 64 bits as well. Might support
     * custom 128 bits in the future.
     * @warning Fixed to 32/single or 64/double precision
     * for floating point.
     */
    uint8_t numBits;

    /**
     * @param inputFormat Input formatting
     * @note Only used for incoming formats. Not
     * applicable to each entry, as that is handled
     * by the entry type
     * 0 = integer base
     * 1 = fixed point
     * 2 = floating point
     * others = Not valid
     */
    uint8_t inputFormat;

    /**
     * @param outputFormat Output formatting
     * @note Only affects which format the result is
     * formatted as.
     * 0 = integer base
     * 1 = fixed point
     * 2 = floating point
     * others = Not valid
     */
    uint8_t outputFormat;

    /**
     * @param sign True if using signed, false if unsigned
     */
    bool sign;

    /**
     * @param inputBase Base of incoming input.
     * @note This can be changed throughout an entry,
     * and if it's changed within a number, that number
     * shall be converted to the new base. E.g.
     * input = 0b10+0b10| ->/change base/->0b10+0x2
     * @warning Fixed to decimal or binary for floating point.
     */
    inputBase_t inputBase;

    /**
     * @param fixedPointDecimalPlace Decimal place for fixed point
     * @note Parameter only valid for fixed point. Denotes
     * how many bits the decimal values hold, i.e. for int this is 0.
     * For e.g. 0b...1000.00 this is 2
     */
    uint8_t fixedPointDecimalPlace;

} numberFormat_t;

#endif
