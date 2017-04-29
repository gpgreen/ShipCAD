/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2015, by Greg Green <ggreen@bit-builder.com>									*
 *    Original Copyright header below															*
 *																								*
 *    This code is distributed as part of the FREE!ship project. FREE!ship is an                *
 *    open source surface-modelling program based on subdivision surfaces and intended for      *
 *    designing ships.                                                                          *
 *                                                                                              *
 *    Copyright © 2005, by Martijn van Engeland                                                 *
 *    e-mail                  : Info@FREEship.org                                               *
 *    FREE!ship project page  : https://sourceforge.net/projects/freeship                       *
 *    FREE!ship homepage      : www.FREEship.org                                                *
 *                                                                                              *
 *    This program is free software; you can redistribute it and/or modify it under             *
 *    the terms of the GNU General Public License as published by the                           *
 *    Free Software Foundation; either version 2 of the License, or (at your option)            *
 *    any later version.                                                                        *
 *                                                                                              *
 *    This program is distributed in the hope that it will be useful, but WITHOUT ANY           *
 *    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A           *
 *    PARTICULAR PURPOSE. See the GNU General Public License for more details.                  *
 *                                                                                              *
 *    You should have received a copy of the GNU General Public License along with              *
 *    this program; if not, write to the Free Software Foundation, Inc.,                        *
 *    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                                     *
 *                                                                                              *
 *##############################################################################################*/

#ifndef POINTERVEC_H_
#define POINTERVEC_H_

#include <vector>
#include <algorithm>

template <class T>
class PointerVector
{
public:

	typedef void apply_fn(T* elem);
	
	PointerVector()
		: _owned(false)
		{}

	PointerVector(bool owned)
		: _owned(owned)
		{}
	
	~PointerVector()
		{clear();}

	void clear()
		{
			if (_owned) {
                for (size_t i=0; i<_vec.size(); i++)
					delete _vec[i];
			}
			_vec.clear();
		}

	size_t size() const
		{return _vec.size();}

	T* get(size_t index)
		{return _vec[index];}

	const T* get(size_t index) const
		{return _vec[index];}
	
	void add(T* elem) {_vec.push_back(elem);}

	bool del(T* elem)
    {
        typename std::vector<T*>::iterator i = std::find(_vec.begin(), _vec.end(), elem);
        return priv_del(i);
    }

	void apply(apply_fn* fn)
		{std::for_each(_vec.begin(), _vec.end(), fn);}

    typename std::vector<T*>::iterator begin()
		{return _vec.begin();}

    typename std::vector<T*>::iterator end()
		{return _vec.end();}
	
    typename std::vector<T*>::const_iterator begin() const
		{return _vec.begin();}

    typename std::vector<T*>::const_iterator end() const
		{return _vec.end();}
	
    void erase(typename std::vector<T*>::iterator i)
        {priv_del(i);}

    void insert(typename std::vector<T*>::iterator position,
                typename std::vector<T*>::iterator first,
                typename std::vector<T*>::iterator last)
    {
        _vec.insert(position, first, last);
    }

private:
    bool priv_del(typename std::vector<T*>::iterator i)
    {
        bool removed = false;
        if (i != _vec.end()) {
            if (_owned)
                delete *i;
            _vec.erase(i);
            removed = true;
        }
        return removed;
    }

	// define copy constructor and assignment operator
	PointerVector(const PointerVector&);
	PointerVector& operator=(const PointerVector&);
	
	bool _owned;
	std::vector<T*> _vec;
};

#endif
