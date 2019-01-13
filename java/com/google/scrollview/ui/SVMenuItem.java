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

import javax.swing.JMenuItem;

/**
 * Constructs a new menulistitem which also has a value and a description. For
 * these, we will not have to ask the server what the value is when the user
 * wants to change it, but can just call the client with the new value.
 */
class SVMenuItem extends SVAbstractMenuItem {
  public String value = null;
  public String desc = null;

  SVMenuItem(int id, String name, String v, String d) {
    super(id, name, new JMenuItem(name));
    value = v;
    desc = d;
  }

  /**
   * Ask the user for new input for a variable and send it.
   * Depending on whether there is a description given for the entry, show
   * the description in the dialog or just show the name.
   */
  @Override
  public void performAction(SVWindow window, SVEventType eventType) {
    if (desc != null) {
      window.showInputDialog(desc, value, id, eventType);
    } else {
      window.showInputDialog(name, value, id, eventType);
    }
  }

  /** Returns the actual value of the MenuListItem. */
  @Override
  public String getValue() {
    return value;
  }
}
