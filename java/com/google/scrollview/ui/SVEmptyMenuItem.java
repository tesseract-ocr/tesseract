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

import javax.swing.JMenuItem;

/**
 * Constructs a new menulistitem which just has an ID and a name attached to
 * it. In this case, we will have to ask for the value of the item and its
 * description if it gets called.
 */
class SVEmptyMenuItem extends SVAbstractMenuItem {
  SVEmptyMenuItem(int id, String name) {
    super(id, name, new JMenuItem(name));
  }
  /** What to do when user clicks on this item. */
  @Override
  public void performAction(SVWindow window, SVEventType eventType) {
  // Send an event indicating that someone clicked on an entry.
  // Value will be null here.
    SVEvent svme =
        new SVEvent(eventType, window, id, getValue());
    ScrollView.addMessage(svme);
  }
}
