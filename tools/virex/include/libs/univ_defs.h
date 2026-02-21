/**
 * @file univ_defs.h
 * @brief Universal definitions for the 16-bit Virtual Gameboy (GBVM) library.
 *
 * This header file contains various universal definitions used in the GBVM library.
 * It includes necessary standard C libraries and defines several constants and types.
 *

 * @authors Soham Metha, Omkar Jagtap
 * @date January 2025
 */

#pragma once

#pragma GCC diagnostic ignored "-Wincompatible-library-redeclaration"

/**
 * @brief Retrieves the next command line argument.
 *
 * This function retrieves the next command line argument from the argument list
 * and updates the argument count and argument list pointers to only contain the
 * remaining pointers.
 *
 * @param argc A pointer to the argument count.
 * @param argv A pointer to the argument list.
 * @return The next command line argument.
 */
char* getNextCmdLineArg(int* argc, char*** argv);

typedef struct Region Region;
