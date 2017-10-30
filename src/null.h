/*
 * AdPlay/UNIX - OPL2 audio player
 * Copyright (C) 2001, 2002 Simon Peter <dn.tlp@gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  
 */

#ifndef H_NULL
#define H_NULL

#include "output.h"

class NullOutput: public Player
{
public:
  NullOutput(Copl *nopl)
    :opl(nopl)
  { }

  virtual void frame() {
    playing = p->update();
    
    CDiskopl *dopl = dynamic_cast<CDiskopl *>(opl);
    if (dopl != NULL) {
      dopl->update(p);
    }
  }

  virtual Copl *get_opl()
    { return opl; }

private:
  Copl *opl;
};

#endif
