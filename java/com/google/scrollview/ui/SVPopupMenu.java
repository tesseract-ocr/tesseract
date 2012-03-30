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

import com.google.scrollview.events.SVEventType;
import com.google.scrollview.ui.SVMenuItem;
import com.google.scrollview.ui.SVWindow;

import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;

import javax.swing.JMenu;
import javax.swing.JPopupMenu;

/**
 * The SVPopupMenu class provides the functionality to add a popup menu to
 * ScrollView. Each popup menu item gets associated with a (client-defined)
 * command-id, which SVPopupMenu will return upon clicking it.
 *
 * @author wanke@google.com
 *
 */

public class SVPopupMenu implements ActionListener {
  /** The root entry to add items to. */
  private JPopupMenu root;
  /** Contains a map of item name to its actual entry. */
  private HashMap<String, SVAbstractMenuItem> items;
  /** The window the menubar belongs to. */
  private SVWindow svWindow;

  /**
   * Create a new SVPopupMenu and associate it with a ScrollView window.
   *
   * @param sv The window our popup menu belongs to.
   */
  SVPopupMenu(SVWindow sv) {
    root = new JPopupMenu();
    svWindow = sv;
    items = new HashMap<String, SVAbstractMenuItem>();
  }

  /**
   * Add a new entry to the menubar. For these items, the server will poll the
   * client to ask what to do.
   *
   * @param parent The menu we add our new entry to (should have been defined
   *        before). If the parent is "", we will add the entry to the root
   *        (top-level)
   * @param name The caption of the new entry.
   * @param id The Id of the new entry. If it is -1, the entry will be treated
   *        as a menu.
   */
  public void add(String parent, String name, int id) {
    // A duplicate entry - we just throw it away, since its already in.
    if (items.get(name) != null) { return; }
    // A new submenu at the top-level
    if (parent.equals("")) {
      JMenu jli = new JMenu(name);
      SVAbstractMenuItem mli = new SVSubMenuItem(name, jli);
      items.put(name, mli);
      root.add(jli);
    }
    // A new sub-submenu
    else if (id == -1) {
      SVAbstractMenuItem jmi = items.get(parent);
      JMenu jli = new JMenu(name);
      SVAbstractMenuItem mli = new SVSubMenuItem(name, jli);
      items.put(name, mli);
      jmi.add(jli);
    }
    // A new child entry. Add to appropriate parent.
    else {
      SVAbstractMenuItem jmi = items.get(parent);
      if (jmi == null) {
        System.out.println("ERROR: Unknown parent " + parent);
        System.exit(1);
      }
      SVAbstractMenuItem mli = new SVEmptyMenuItem(id, name);
      mli.mi.addActionListener(this);
      items.put(name, mli);
      jmi.add(mli);
    }
  }

  /**
   * Add a new entry to the menubar. In this case, we also know its value and
   * possibly even have a description. For these items, the server will not poll
   * the client to ask what to do, but just show an input dialog and send a
   * message with the new value.
   *
   * @param parent The menu we add our new entry to (should have been defined
   *        before). If the parent is "", we will add the entry to the root
   *        (top-level)
   * @param name The caption of the new entry.
   * @param id The Id of the new entry. If it is -1, the entry will be treated
   *        as a menu.
   * @param value The value of the new entry.
   * @param desc The description of the new entry.
   */
  public void add(String parent, String name, int id, String value, String desc) {
    SVAbstractMenuItem jmi = items.get(parent);
    SVMenuItem mli = new SVMenuItem(id, name, value, desc);
    mli.mi.addActionListener(this);
    items.put(name, mli);
    if (jmi == null) { // add to root
      root.add(mli.mi);
    } else { // add to parent
      jmi.add(mli);
    }
  }



  /**
   * A click on one of the items in our menubar has occured. Forward it
   * to the item itself to let it decide what happens.
   */
  public void actionPerformed(ActionEvent e) {

    // Get the corresponding menuitem
    SVAbstractMenuItem svm = items.get(e.getActionCommand());

   svm.performAction(svWindow, SVEventType.SVET_POPUP);
  }

  /**
   * Gets called by the SVEventHandler of the window to actually show the
   * content of the popup menu.
   */
  public void show(Component Invoker, int x, int y) {
    root.show(Invoker, x, y);
  }
}
