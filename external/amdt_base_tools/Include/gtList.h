//=====================================================================
// Copyright 2016-2025 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gtList.h
///
//=====================================================================

//------------------------------ gtList.h ------------------------------

#ifndef __GTLIST
#define __GTLIST

// STL:
#include <list>

// Infra:
#include <amdt_base_tools/Include/AMDTDefinitions.h>

// Local:
#include <amdt_base_tools/Include/gtIgnoreCompilerWarnings.h>


// ----------------------------------------------------------------------------------
// Class Name:           gtList
// General Description:
//  A class representing a list of elements. It is implemented as a doubly linked list.
//  This enables:
//  a. Efficient forward and backward traversal on the list items.
//  b. Constant time insertion and removal of elements at the beginning or the end of the list.
//
//  Notice:
//  a. Insertion and splicing do not invalidate iterators to list elements.
//  b. Removal invalidates only the iterators that point to the elements that are removed.
//
// Author:      AMD Developer Tools Team
// Creation Date:        11/5/2003
// ----------------------------------------------------------------------------------
template<typename _Tp, typename _Alloc = std::allocator<_Tp> >
class gtList : public std::list<_Tp, _Alloc>
{
public:
    typedef std::list<_Tp, _Alloc> StdList;
    gtList() {};
    gtList(const gtList& other) : StdList(other) {};

    gtList& operator=(const gtList& other)
    {
        static_cast<StdList*>(this)->operator=(other);
        return *this;
    }

#if AMDT_HAS_CPP0X
    gtList(gtList&& other) : StdList(std::move(other)) {}

    gtList& operator=(gtList&& other)
    {
        static_cast<StdList*>(this)->operator=(std::move(other));
        return *this;
    }
#endif


    // ---------------------------------------------------------------------------
    // Name:        length
    // Description: Returns the list length.
    //              Notice that this function complexity is O(n).
    // Author:      AMD Developer Tools Team
    // Date:        8/5/2005
    // ---------------------------------------------------------------------------
    int length() const
    {
        int itemsAmount = 0;

        // Count the list items:
        typename StdList::const_iterator endIter = gtList::end();
        typename StdList::const_iterator iter = gtList::begin();

        while (iter != endIter)
        {
            itemsAmount++;
            iter++;
        }

        return itemsAmount;
    }
};


#endif  // __GTLIST
