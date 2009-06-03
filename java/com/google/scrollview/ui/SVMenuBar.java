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
import com.google.scrollview.ui.SVWindow;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;

import javax.swing.JMenu;
import javax.swing.JMenuBar;

/**
 * The SVMenuBar class provides the functionality to add a menubar to
 * ScrollView. Each menubar item gets associated with a (client-defined)
 * command-id, which SVMenuBar will return upon clicking it.
 *
 * @author wanke@google.com
 *
 */
public class SVMenuBar implements ActionListener {
  /** The root entry to add items to. */
  private JMenuBar root;
  /** Contains a map of item name to its actual entry. */
  private HashMap<String, SVAbstractMenuItem> items;
  /** The window the menubar belongs to. */
  private SVWindow svWindow;

  /**
   * Create a new SVMenuBar and place it at the top of the ScrollView window.
   *
   * @param scrollView The window our menubar belongs to.
   */
  public SVMenuBar(SVWindow scrollView) {
    root = new JMenuBar();
    svWindow = scrollView;
    items = new HashMap<String, SVAbstractMenuItem>();
    svWindow.setJMenuBar(root);
  }


  /**
   * A click on one of the items in our menubar has occured. Forward it
   * to the item itself to let it decide what happens.
   */
  public void actionPerformed(ActionEvent e) {
    // Get the corresponding menuitem.
    SVAbstractMenuItem svm = items.get(e.getActionCommand());

    svm.performAction(svWindow, SVEventType.SVET_MENU);
  }

  /**
   * Add a new entry to the menubar.
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
   * Add a new checkbox entry to the menubar.
   *
   * @param parent The menu we add our new entry to (should have been defined
   *        before). If the parent is "", we will add the entry to the root
   *        (top-level)
   * @param name The caption of the new entry.
   * @param id The Id of the new entry. If it is -1, the entry will be treated
   *        as a menu.
   * @param b Whether the entry is initally flagged.
   *
   */

  public void add(String parent, String name, int id, boolean b) {
    SVAbstractMenuItem jmi = items.get(parent);
    if (jmi == null) {
      System.out.println("ERROR: Unknown parent " + parent);
      System.exit(1);
    }
    SVAbstractMenuItem mli = new SVCheckboxMenuItem(id, name, b);
    mli.mi.addActionListener(this);
    items.put(name, mli);
    jmi.add(mli);
  }

}
