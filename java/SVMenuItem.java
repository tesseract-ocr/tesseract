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
   * @param eventType What kind of event will be associated (usually SVET_POPUP or SVET_MENU).
   */
  public void performAction(SVWindow window, SVEventType eventType) {} 
}

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

