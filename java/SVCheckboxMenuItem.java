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

import com.google.scrollview.ScrollView;
import com.google.scrollview.events.SVEvent;
import com.google.scrollview.events.SVEventType;

import javax.swing.JCheckBoxMenuItem;

/**
 * Constructs a new menulistitem which possesses a flag that can be toggled.
 */
class SVCheckboxMenuItem extends SVAbstractMenuItem {
  public String value = null;
  public String desc = null;
  public boolean bvalue;

  SVCheckboxMenuItem(int id, String name, boolean val) {
    super(id, name, new JCheckBoxMenuItem(name, val));
    bvalue = val;
  }

  /** What to do when user clicks on this item. */
  @Override
  public void performAction(SVWindow window, SVEventType eventType) {
    // Checkbox entry - trigger and send event.
    if (bvalue) {
      bvalue = false;
    } else {
      bvalue = true;
    }
    SVEvent svme = new SVEvent(eventType, window, id, getValue());
    ScrollView.addMessage(svme);
  }

  /** Returns the actual value of the MenuListItem. */
  @Override
  public String getValue() {
    return Boolean.toString(bvalue);
  }
}

