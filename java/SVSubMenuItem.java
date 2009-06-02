// Copyright 2007 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); You may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by
// applicable law or agreed to in writing, software distributed under the
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied. See the License for the specific
// language governing permissions and limitations under the License.

package com.google.scrollview.ui;

/**
 * A MenuListItem is any sort of menu entry. This can either be within a popup
 * menu or within a menubar. It can either be a submenu (only name and
 * command-id) or a name with an associated value and possibly description. They
 * can also have new entries added (if they are submenus).
 *
 * @author wanke@google.com
 */

import javax.swing.JMenu;

/** Constructs a new submenu which can hold other entries. */
class SVSubMenuItem extends SVAbstractMenuItem {
  public SVSubMenuItem(String name, JMenu jli) {
    super(-1, name, jli);
  }
  /** Adds a child entry to the submenu. */
  @Override
  public void add(SVAbstractMenuItem mli) {
    mi.add(mli.mi);
  }
  /** Adds a child menu to the submenu (or root node). */
  @Override
  public void add(JMenu jli) {
    mi.add(jli);
  }
}


