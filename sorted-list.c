#include <stdio.h>
#include "sorted-list.h"

/*
 * NCreate creates a new Node that holds newObj, points to n, 
 * and contains the DestructFuncT df. DestructFuncT df must be stored in 
 * the node in case the list is destroyed before the node is.
 */
NodePtr NCreate( void * newObj, DestructFuncT df, NodePtr n ) {
  NodePtr node = ( NodePtr ) malloc( sizeof( struct Node ) );
  node->data = newObj;
  node->next = n;
  node->numPointers = 0;
  node->df = df;
  return node;
}

/*
 * NDestroy destroy's the node's data with the saved df function and 
 * frees the node.
 */
void NDestroy( NodePtr n ) {
  n->df( n->data );
  free( n );
}

/*
 * SLCreate creates a sorted list that uses the CompareFuncT cf, 
 * DestructFuncT df, and points to NULL.
 */
SortedListPtr SLCreate( CompareFuncT cf, DestructFuncT df ) {
  SortedListPtr list = ( SortedListPtr ) malloc( sizeof( struct SortedList ) );
  list->cf = cf;
  list->df = df;
  list->head = NULL;
}

/*
 * SLDestroy makes all the nodes in the list point to NULL, decrements
 * the nodes' numPointers int, and destroys any node whose numPointers is
 * less than or equal to zero. It finally frees the list.
 */
void SLDestroy( SortedListPtr list ) {
  NodePtr prev = NULL;
  NodePtr current = list->head;
  list->head = NULL;
  while ( current != NULL ) {
    current->numPointers--;
    prev = current;
    current = prev->next;
    prev->next = NULL;
    if ( prev->numPointers <= 0 ) {
      NDestroy( prev );
    }
  }
  free( list );
}

/*
 * SLInsert looks through the list until it finds the end of the list or 
 * a value that is less that newObj. If the value of newObj is already 
 * in the list, then SLInsert tells the caller that they're an idiot and 
 * returns zero. If the value of newObj is valid, it is added to the list.
 * If it was inserted at the beginning of the list, the list's head pointer 
 * points to it. Otherwise, the preceeding node points to the node that was 
 * just added. Finally, the recently added node's numPointers variable 
 * is incremented.
 */
int SLInsert( SortedListPtr list, void *newObj ) {
  NodePtr prev = NULL;
  NodePtr current = list->head;
  while ( current != NULL && list->cf(current->data, newObj) < 0 ) {
      prev = current;
      current = current->next;
  }
  NodePtr n = NCreate( newObj, list->df, current );
  if ( prev == NULL ) {
    list->head = n;
  } else {
    prev->next = n;
  }
  n->numPointers++;
  return 1;
}

/*
 * SLRemove first finds newObj in the list. If it can't be found, the function 
 * returns zero. The node containing the value of newObj is removed from 
 * the list and it's numPointers variable is decremented. If the node's 
 * numPointers is less than or equal to zero, it is destroyed.
 */
int SLRemove( SortedListPtr list, void *newObj ) {
  NodePtr prev = NULL;
  NodePtr current = list->head;
  while ( current != NULL && list->cf(current->data, newObj) != 0 ) {
    prev = current;
    current = current->next;
  }
  if ( current == NULL ) {
    printf( "Item not found in list.\n" );
    return 0;
  }
  if ( prev == NULL ) {
    list->head = current->next;
  } else {
    prev->next = current->next;
  }
  current->numPointers--;
  if ( current->numPointers <= 0 ) {
    NDestroy( current );
  }
  return 1;
}

/*
 * SLCreateIterator creates an iterator that starts out pointing to the 
 * list's head node.
 */
SortedListIteratorPtr SLCreateIterator( SortedListPtr list ) {
  SortedListIteratorPtr iter = ( SortedListIteratorPtr ) malloc( sizeof( struct SortedListIterator ) );
  iter->currentNode = list->head;
  iter->currentNode->numPointers++;
  return iter;
}

/*
 * SLDestroyIterator decrements its current node if that node isn't NULL.
 * It then destroys the node if it's numPointers is less than or equal to zero.
 */
void SLDestroyIterator( SortedListIteratorPtr iter ) {
  if ( iter->currentNode != NULL ) {
    iter->currentNode->numPointers--;
    if ( iter->currentNode->numPointers <= 0 ) {
      NDestroy( iter->currentNode );
    }
  }
  free( iter );
}

/*
 * SLGetItem returns the value of the node that the iterator is 
 * currently pointing to.
 */
void * SLGetItem( SortedListIteratorPtr iter ) {
  return iter->currentNode->data;
}

/*
 * SLNextItem first checks if the node it is pointing to is NULL and 
 * returns NULL if the value is NULL. If the function is still going, 
 * it then decrement's the current node's numPointers variable and destroys it 
 * if numPointers is less than or equal to zero. The iterator then points to 
 * the next node. If the node is NULL, the function immediately returns NULL. 
 * If it is not NULL, the iterator increments the node's numPointers variable 
 * and returns the value of that node using SLGetItem.
 */
void * SLNextItem( SortedListIteratorPtr iter ) {
  if ( iter->currentNode == NULL ) {
    return NULL;
  }
  iter->currentNode->numPointers--;
  NodePtr next = iter->currentNode->next;
  if ( iter->currentNode->numPointers <= 0 ) {
    NDestroy( iter->currentNode );
  }
  iter->currentNode = next;
  if ( iter->currentNode == NULL) {
    return NULL;
  }
  iter->currentNode->numPointers++;
  return SLGetItem( iter );
}