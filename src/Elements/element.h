/* EPANET 3
 *
 * Copyright (c) 2016 Open Water Analytics
 * Licensed under the terms of the MIT License (see the LICENSE file for details).
 *
 */

//! \file element.h
//! \brief Describes the Element class.

#ifndef ELEMENT_H_
#define ELEMENT_H_

#include <string>

//! \class Element
//! \brief Abstract parent class for all pipe network components.

class Element
{
  public:

    enum ElementType {NODE, LINK, PATTERN, CURVE, CONTROL};

    Element(std::string name_);
    virtual ~Element() = 0;

    std::string name;       //!< element's ID name
    int         index;      //!< index in array of elements

  private:

    // Elements can't be copied or tested for equality
    Element(const Element& e);
    Element& operator=(const Element& e);
};

#endif
