/*###############################################################################################
 *    ShipCAD																					*
 *    Copyright 2016, by Greg Green <ggreen@bit-builder.com>									*
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

#ifndef GRID_H_
#define GRID_H_

#include <vector>
#include <stdexcept>

namespace ShipCAD {

//////////////////////////////////////////////////////////////////////////////////////

template <typename T> class Grid
{
public:
    std::vector<std::vector<T> > grid;

    explicit Grid() {}
    
    explicit Grid(size_t rows, size_t cols)
        {
            setRows(rows);
            setCols(cols);
        }

    explicit Grid(size_t rows, size_t cols, const T& defval)
    {
        setRows(rows);
        for (size_t i=0; i<rows; ++i)
            grid[i].assign(cols, defval);
    }
    
    size_t cols() const {return grid.size() > 0 ? grid[0].size() : 0;}
    size_t rows() const {return grid.size();}
    
    void setRows(size_t rows) 
        {
            grid.resize(rows);
        }

    void setCols(size_t cols)
        {
            if (grid.size() == 0)
                throw std::overflow_error("can't set column size when rows are 0");
            for (size_t i=0; i<grid.size(); i++)
                grid[i].resize(cols);
        }
    
    const T& get(size_t row, size_t col) const {
        if (row >= grid.size())
            throw std::overflow_error("grid row out of bounds");
        if (col >= grid[0].size())
            throw std::overflow_error("grid col out of bounds");
        return grid[row][col];
    }

    void set(size_t row, size_t col, T elem) {
        if (row >= grid.size())
            throw std::overflow_error("grid row out of bounds");
        if (col >= grid[0].size())
            throw std::overflow_error("grid col out of bounds");
        grid[row][col] = elem;
    }

    void setWithExpansion(size_t row, size_t col, T elem, T defaultval) 
        {
            while (row >= grid.rows())
            {
                std::vector<T> newrow(cols(), defaultval);
                grid.push_back(newrow);
            }
            while (col >= grid.cols())
            {
                for (size_t i=0; i<grid.size(); ++i)
                    grid.push_back(defaultval);
            }
    
            grid[row][col] = elem;
        }

    void deleteColumn(size_t col)
        {
            if (grid.size() == 0 || col > grid[0].size())
                throw std::overflow_error("grid col out of bounds");
            for(size_t i=0; i<grid.size(); ++i)
                grid[i].erase(grid[i].begin() + col);
        }

    void deleteRow(size_t row)
        {
            if (row > grid.size())
                throw std::overflow_error("grid row out of bounds");
            grid.erase(grid.begin() + row);
        }

    void clear()
        {
            for (size_t i=0; i<grid.size(); ++i)
                grid[i].clear();
            grid.clear();
        }
};

};				/* end namespace */

//////////////////////////////////////////////////////////////////////////////////////

#endif

