#include "appwindow.hpp"
#include <string>

AppWindow::AppWindow() {
  m_viewer = new Viewer(this);

  set_title("CS488 Assignment Two, by Alex Klen");

  //using Gtk::Menu_Helpers::MenuElem;

  // Application Menu.
  Gtk::MenuItem *reset = Gtk::manage(new Gtk::MenuItem("_Reset", true));
  Gtk::MenuItem *quit = Gtk::manage(new Gtk::MenuItem("_Quit", true));

  reset->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::reset_view));
  quit->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::hide));

  add_accelerator(reset, 'a');
  add_accelerator(quit, 'q');

  m_menu_app.items().push_back(*reset);
  m_menu_app.items().push_back(*quit);

  // Mode Menu.
  const char shortcuts[] = {
    'o', 'n', 'p', 'r', 't', 's'
  };
  const std::string names[] = {
    "R_otate View",
    "Tra_nslate View",
    "_Perspective",
    "_Rotate Model",
    "_Translate Model",
    "_Scale Model"
  };

  Gtk::RadioMenuItem::Group view_mode_group;
  sigc::slot1<void, Viewer::Mode> view_mode_slot = sigc::mem_fun(*m_viewer, &Viewer::set_mode);

  for (int mode = 0; mode < Viewer::NUM_MODES; mode++) {
    Gtk::RadioMenuItem *rb = Gtk::manage(new Gtk::RadioMenuItem(view_mode_group, names[mode], true));
    add_accelerator(rb, shortcuts[mode]);
    rb->signal_activate().connect(sigc::bind(view_mode_slot, (Viewer::Mode) mode));
    m_menu_mode.items().push_back(*rb);
  }

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Mode", m_menu_mode));

  // Select default radio button.
  m_menu_mode.items()[Viewer::DEFAULT_MODE].activate();

  // Pack in our widgets

  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
  m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

  // Put the viewer below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_viewer->set_size_request(300, 300);
  m_vbox.pack_start(*m_viewer);

  m_vbox.pack_start(m_main_label, false, false);

  show_all();
}

AppWindow::~AppWindow() {
  delete m_viewer;
}

void AppWindow::add_accelerator(Gtk::MenuItem *it, char accelerator) {
  it->add_accelerator("activate", get_accel_group(), accelerator, (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  it->add_accelerator("activate", get_accel_group(), accelerator, Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
}

void AppWindow::redraw_label(int mode, double fov, double near, double far) {
  const char* names[] = {
    "Rotate View",
    "Translate View",
    "Perspective",
    "Rotate Model",
    "Translate Model",
    "Scale Model"
  };
  char buf[255];
  sprintf(buf, "Mode: %s, Field of View = %.1f°, near plane = %.1f, far plane = %.1f", names[mode], fov, near, far);
  m_main_label.set_label(buf);
}

void AppWindow::reset_view() {
  m_viewer->reset_view();
  m_menu_mode.items()[Viewer::DEFAULT_MODE].activate();
}

