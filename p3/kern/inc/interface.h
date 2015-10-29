/** @file interface.h
 *
 *  @brief Interface to access the data structures
 *
 *  @author Jonathan Ong (jonathao) and Evan Palmer (esp)
 **/
 
#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <variable_queue.h>

/** @def NEW_STRUCT(STRUCT_TYPE, ELEM_TYPE)
 *
 *  @brief Generates a new structure of type STRUCT_TYPE containing elements of
 *         type ELEM_TYPE.
 *
 *  @param STRUCT_TYPE the type you wish the newly-generated structure to have
 *
 *  @param Q_ELEM_TYPE the type of elements stored in the queue.
 *         Q_ELEM_TYPE must be a structure.
 **/
#define NEW_STRUCT(STRUCT_TYPE, ELEM_TYPE) \
    Q_NEW_HEAD(STRUCT_TYPE, ELEM_TYPE)

/** @def NEW_LINK(STRUCT_TYPE, ELEM_TYPE)
 *
 *  @brief Instantiates elements necessary to index elements in the struct
 *
 *  @param STRUCT_TYPE the type you wish the newly-generated structure to have
 **/
#define NEW_LINK(ELEM_TYPE) \
    Q_NEW_LINK(ELEM_TYPE)

/** @def INIT_STRUCT(STRUCT)
 *
 *  @brief Initializes the data structure for use.
 *  
 *  @param STRUCT Pointer to the data structure
 **/
#define INIT_STRUCT(STRUCT) \
    Q_INIT_HEAD(STRUCT)
    
/** @def INIT_ELEM(ELEM, LINK_NAME)
 *
 *  @brief Initializes the link named LINK_NAME in an instance of the structure
 *         ELEM.
 *
 *  @param ELEM Pointer to the structure instance containing the link
 *  @param LINK_NAME The name of the link to initialize
 **/
#define INIT_ELEM(ELEM, LINK_NAME) \
    Q_INIT_ELEM(ELEM, LINK_NAME)

/** @def INSERT(STRUCT, ELEM, LINK_NAME)
 *
 *  @brief Inserts the element pointed to by ELEM into the structure STRUCT.
 *
 *  @param STRUCT Pointer to the data structure into which ELEM will be
 *         inserted
 *  @param ELEM Pointer to the element to insert
 *  @param LINK_NAME Name of the link used to organize the data structure
 **/
#define INSERT(STRUCT, ELEM, LINK_NAME) \
    Q_INSERT_FRONT(STRUCT, ELEM, LINK_NAME)

/** @def REMOVE(STRUCT, ELEM, LINK_NAME)
 *
 *  @brief Removes the element pointed to by ELEM from the structure STRUCT.
 *
 *  @param STRUCT Pointer to the data structure from which ELEM will be
 *         removed
 *  @param ELEM Pointer to the element to remove
 *  @param LINK_NAME Name of the link used to organize the data structure
 **/
#define REMOVE(STRUCT, ELEM, LINK_NAME) \
    Q_REMOVE(STRUCT, ELEM, LINK_NAME)

#endif // INTERFACE_H_
    
