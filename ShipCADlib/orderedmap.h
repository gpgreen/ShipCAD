/*##############################################################################################
 *    ShipCAD                                                                                  *
 *    Copyright 2017, by Greg Green <ggreen@bit-builder.com>                                   *
 *    Original Copyright header below                                                          *
 *                                                                                             *
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an               *
 *    open source surface-modelling program based on subdivision surfaces and intended for     *
 *    designing ships.                                                                         *
 *                                                                                             *
 *    Copyright © 2005, by Martijn van Engeland                                                *
 *    e-mail                  : Info@FREEship.org                                              *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                      *
 *    FREE!ship homepage      : www.FREEship.org                                               *
 *                                                                                             *
 *    This program is free software; you can redistribute it and/or modify it under            *
 *    the terms of the GNU General Public License as published by the                          *
 *    Free Software Foundation; either version 2 of the License, or (at your option)           *
 *    any later version.                                                                       *
 *                                                                                             *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY          *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A          *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                 *
 *                                                                                             *
 *    You should have received a copy of the GNU General Public License along with             *
 *    this program; if not, write to the Free Software Foundation, Inc.,                       *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                    *
 *                                                                                             *
 *#############################################################################################*/

#ifndef ORDERED_MAP_H_
#define ORDERED_MAP_H_

#include <algorithm>
#include <stdexcept>
#include <vector>
#include <map>

namespace ShipCAD {
    class SubdivisionControlPoint;
}

// forward declaration
class OrderedPointMap;

//////////////////////////////////////////////////////////////////////////////////////

/*! \brief Iterator class for OrderedPointMap
 */
class OrderedPointMapIterator
{
public:
    // constructor
    explicit OrderedPointMapIterator(const std::map<ShipCAD::SubdivisionControlPoint*, size_t>::iterator& iter)
        : _iter(iter) {}
    // destructor
    ~OrderedPointMapIterator() {}

    // copy constructor
    OrderedPointMapIterator(const OrderedPointMapIterator& other)
        : _iter(other._iter) {}

    // assignment operator
    OrderedPointMapIterator& operator=(const OrderedPointMapIterator& other)
    {
        if (other == *this)
            return *this;
        _iter = other._iter;
        return *this;
    }

    // increment operator
    OrderedPointMapIterator& operator++()
    {
        ++_iter;
        return *this;
    }

    // equal operator
    bool operator==(const OrderedPointMapIterator& other) const
    {
        return _iter == other._iter;
    }

    // non equal operator
    bool operator!=(const OrderedPointMapIterator& other) const
    {
        return _iter != other._iter;
    }

    // dereference operator
    ShipCAD::SubdivisionControlPoint* operator*()
    {
        return (*_iter).first;
    }

    // get the index of this point
    size_t getIndex() const
    {
        return (*_iter).second;
    }
    
private:
    
    std::map<ShipCAD::SubdivisionControlPoint*, size_t>::iterator _iter;
    
};

/*! \brief class to keep collection of SubdivisionControlPoints and associated index
 *
 * When selecting points, we need to keep track of the order they are selected. If
 * we just use a vector though, we have linear lookup to see if it belongs to the
 * selected set. This class primarily stores the points in a set, so we get O(1)
 * lookup of point membership, and O(n) lookup by order. Since we only check for
 * order when doing a user operation, this is the desired behaviour
 *
 * If a point is already in the set, the currently selected order of
 * the points is not changed nor is the point in the set more than
 * once.
 */
class OrderedPointMap
{
    
public:
    typedef OrderedPointMapIterator iterator;

    // constructor
    explicit OrderedPointMap() : _bag() {}
    // destructor
    ~OrderedPointMap() {}

    size_t size() const
    {
        return _bag.size();
    }
    
    void clear()
    {
        _bag.clear();
    }
    
    void add(ShipCAD::SubdivisionControlPoint* point)
    {
        std::map<ShipCAD::SubdivisionControlPoint*, size_t>::iterator i = _bag.find(point);
        if (i != _bag.end())
            return;
        _bag.insert(std::make_pair(point, _bag.size()));
    }

    bool has(const ShipCAD::SubdivisionControlPoint* point) const
    {
        OrderedPointMap* us = const_cast<OrderedPointMap*>(this);
        std::map<ShipCAD::SubdivisionControlPoint*, size_t>::iterator i
                = us->_bag.find(const_cast<ShipCAD::SubdivisionControlPoint*>(point));
        return i != us->_bag.end();
    }

    ShipCAD::SubdivisionControlPoint* get(size_t index) const
    {
        std::map<ShipCAD::SubdivisionControlPoint*, size_t>::const_iterator i = _bag.begin();
        for ( ; i!=_bag.end(); i++) {
            if ((*i).second == index)
                return (*i).first;
        }
        throw std::range_error("index out of range in get via index");
    }

    size_t get(const ShipCAD::SubdivisionControlPoint* point) const
    {
        std::map<ShipCAD::SubdivisionControlPoint*, size_t>::const_iterator i = _bag.begin();
        for ( ; i!=_bag.end(); i++) {
            if ((*i).first == point)
                return (*i).second;
        }
        throw std::range_error("index out of range in get via point");
    }
    
    void remove(ShipCAD::SubdivisionControlPoint* point)
    {
        std::map<ShipCAD::SubdivisionControlPoint*, size_t>::iterator i = _bag.find(point);
        if (i == _bag.end())
            return;
        _bag.erase(i);
        reorder();
    }

    iterator begin() {return OrderedPointMapIterator(_bag.begin());}
    iterator end() {return OrderedPointMapIterator(_bag.end());}
    
            
protected:

    // after removing an element, reorder the indexes to fix gaps
    void reorder()
    {
        // make a list of indexes in order
        std::vector<size_t> indexes;
        size_t cur = 0;
        while (indexes.size() < _bag.size()) {
            std::map<ShipCAD::SubdivisionControlPoint*, size_t>::const_iterator i = _bag.begin();
            for ( ; i!=_bag.end(); i++) {
                if ((*i).second == cur) {
                    indexes.push_back(cur);
                    break;
                }
            }
            cur++;
        }
        for (size_t j=0; j<indexes.size(); j++) {
            if (j == indexes[j])
                continue;
            // index is out of order, reset it
            std::map<ShipCAD::SubdivisionControlPoint*, size_t>::iterator k = _bag.begin();
            for ( ; k!=_bag.end(); k++) {
                if ((*k).second == indexes[j])
                    break;
            }
            if (k == _bag.end())
                throw std::range_error("index out of range in reorder");
            // set the new value
            (*k).second = j;
        }
    }
    
private:
    
    std::map<ShipCAD::SubdivisionControlPoint*, size_t> _bag;

    friend class OrderedPointMapIterator;
};

//////////////////////////////////////////////////////////////////////////////////////

#endif  // ORDERED_MAP_H_
