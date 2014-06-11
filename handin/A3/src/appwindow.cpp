#include "appwindow.hpp"

#define BLINK_DELAY 500

AppWindow::AppWindow(SceneNode* scene): m_viewer(scene) {
  set_title("Advanced Ergonomics Laboratory, by Alex Klen");

  using Gtk::Menu_Helpers::MenuElem;

  // Application Menu.
  const char reset_shortcuts[] = {
    'i', 'o', 'n', 'a'
  };
  const std::string reset_names[] = {
    "Reset _Position",
    "Reset _Orientation",
    "Reset Joi_nts",
    "Reset _All"
  };
  sigc::slot1<void, Viewer::ResetType> reset_slot = sigc::mem_fun(*this, &AppWindow::reset);

  for (int resetType = 0; resetType < Viewer::NUM_RESET_TYPES; resetType++) {
    Gtk::MenuItem *mi = Gtk::manage(new Gtk::MenuItem(reset_names[resetType], true));
    add_accelerator(mi, reset_shortcuts[resetType]);
    mi->signal_activate().connect(sigc::bind(reset_slot, (Viewer::ResetType) resetType));
    m_menu_app.items().push_back(*mi);
  }

  Gtk::MenuItem *quit = Gtk::manage(new Gtk::MenuItem("_Quit", true));
  add_accelerator(quit, 'q');
  quit->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::hide));
  m_menu_app.items().push_back(*quit);


  // Mode Menu.
  const char mode_shortcuts[] = {
    'p', 'j'
  };
  const std::string mode_names[] = {
    "_Position/Orientation",
    "_Joints"
  };

  // Create and wire signals for radio buttons for each mode.
  Gtk::RadioMenuItem::Group view_mode_group;
  sigc::slot1<void, Viewer::Mode> view_mode_slot = sigc::mem_fun(m_viewer, &Viewer::setMode);

  for (int mode = 0; mode < Viewer::NUM_MODES; mode++) {
    Gtk::RadioMenuItem *rb = Gtk::manage(new Gtk::RadioMenuItem(view_mode_group, mode_names[mode], true));
    add_accelerator(rb, mode_shortcuts[mode]);
    rb->signal_activate().connect(sigc::bind(view_mode_slot, (Viewer::Mode) mode));
    m_menu_mode.items().push_back(*rb);
  }

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Mode", m_menu_mode));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Edit", m_menu_edit));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Options", m_menu_options));

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

  // Setup refresh for blinking picked geometry.
  Glib::signal_timeout().connect(sigc::mem_fun(m_viewer, &Viewer::blink), BLINK_DELAY);
}

void AppWindow::add_accelerator(Gtk::MenuItem *it, char accelerator) {
  it->add_accelerator("activate", get_accel_group(), accelerator, (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  it->add_accelerator("activate", get_accel_group(), accelerator, Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
}

void AppWindow::reset(Viewer::ResetType r) {
  m_viewer.reset(r);
  if (r == Viewer::RESET_ALL) {
    m_viewer.setMode(Viewer::DEFAULT_MODE);
    m_menu_mode.items()[Viewer::DEFAULT_MODE].activate();
  }
}

