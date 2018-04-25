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

import com.google.scrollview.events.SVEventType;

import javax.swing.JMenu;
import javax.swing.JMenuItem;

abstract class SVAbstractMenuItem {
  JMenuItem mi;
  public String name;
  public int id;

  /**
   * Sets the basic attributes for name, id and the corresponding swing item
   */
  SVAbstractMenuItem(int id, String name, JMenuItem jmi) {
    this.mi = jmi;
    this.name = name;
    this.id = id;
  }

  /** Returns the actual value of the MenuListItem. */
  public String getValue() { return null; }

  /** Adds a child entry to the submenu. */
  public void add(SVAbstractMenuItem mli) { }

  /** Adds a child menu to the submenu (or root node). */
  public void add(JMenu jli) { }

  /**
   * What to do when user clicks on this item.
   * @param window The window the event happened.
   * @param eventType What kind of event will be associated
   * (usually SVET_POPUP or SVET_MENU).
   */
  public void performAction(SVWindow window, SVEventType eventType) {}
}
