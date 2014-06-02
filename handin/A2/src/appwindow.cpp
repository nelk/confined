#include "appwindow.hpp"

AppWindow::AppWindow() {
  set_title("CS488 Assignment Two");

  // A utility class for constructing things that go into menus, which
  // we'll set up next.
  using Gtk::Menu_Helpers::MenuElem;

  Gtk::MenuItem *quit = Gtk::manage(new Gtk::MenuItem("_Quit", true));
  quit->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::hide));
  add_accelerator(quit, 'q');

  m_menu_app.items().push_back(*quit);

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));

  // Pack in our widgets

  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
  m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

  // Put the viewer below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_viewer.set_size_request(300, 300);
  m_vbox.pack_start(m_viewer);

  show_all();
}

void AppWindow::add_accelerator(Gtk::MenuItem *it, char accelerator) {
  it->add_accelerator("activate", get_accel_group(), accelerator, (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  it->add_accelerator("activate", get_accel_group(), accelerator, Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
}

