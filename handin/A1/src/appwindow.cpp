#include "appwindow.hpp"
#include <iostream>

#define SLOW_TICK_DELAY 500
#define MEDIUM_TICK_DELAY 250
#define FAST_TICK_DELAY 100
#define REFRESH_DELAY 30
#define GAME_COLUMNS 10
#define GAME_ROWS 20

AppWindow::AppWindow() : m_tick_delay(SLOW_TICK_DELAY) {
  set_title("488 Tetrominoes on the Wall, by Alex Klen");
  resize(800, 600);

  m_game = new Game(GAME_COLUMNS, GAME_ROWS);
  m_viewer = new Viewer(m_game);

  // A utility class for constructing things that go into menus, which
  // we'll set up next.
  using Gtk::Menu_Helpers::MenuElem;

  // Application Menu.
  Gtk::MenuItem *newgame = Gtk::manage(new Gtk::MenuItem("_New Game", true));
  Gtk::MenuItem *reset = Gtk::manage(new Gtk::MenuItem("_Reset", true));
  Gtk::MenuItem *quit = Gtk::manage(new Gtk::MenuItem("_Quit", true));

  newgame->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::newgame));
  reset->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::reset));
  quit->signal_activate().connect(sigc::mem_fun(*this, &AppWindow::hide));

  add_accelerator(newgame, 'n');
  add_accelerator(reset, 'r');
  add_accelerator(quit, 'q');

  m_menu_app.items().push_back(*newgame);
  m_menu_app.items().push_back(*reset);
  m_menu_app.items().push_back(*quit);

  // Draw Mode Menu.
  Gtk::RadioMenuItem::Group view_mode_group;
  Gtk::RadioMenuItem *rb_wireframe = Gtk::manage(new Gtk::RadioMenuItem(view_mode_group, "_Wire-frame", true));
  Gtk::RadioMenuItem *rb_faces = Gtk::manage(new Gtk::RadioMenuItem(view_mode_group, "_Face", true));
  Gtk::RadioMenuItem *rb_multicoloured = Gtk::manage(new Gtk::RadioMenuItem(view_mode_group, "_Multicoloured", true));

  add_accelerator(rb_wireframe, 'w');
  add_accelerator(rb_faces, 'f');
  add_accelerator(rb_multicoloured, 'm');

  sigc::slot1<void, Viewer::ViewMode> view_mode_slot = sigc::mem_fun(m_viewer, &Viewer::setViewMode);
  rb_wireframe->signal_activate().connect(sigc::bind(view_mode_slot, Viewer::WIREFRAME));
  rb_faces->signal_activate().connect(sigc::bind(view_mode_slot, Viewer::FACES));
  rb_multicoloured->signal_activate().connect(sigc::bind(view_mode_slot, Viewer::MULTICOLOUR));

  m_menu_draw_mode.items().push_back(*rb_wireframe);
  m_menu_draw_mode.items().push_back(*rb_faces);
  m_menu_draw_mode.items().push_back(*rb_multicoloured);

  // Game speed menu.
  Gtk::RadioMenuItem::Group speed_group;
  Gtk::RadioMenuItem *rb_slow = Gtk::manage(new Gtk::RadioMenuItem(speed_group, "Slow"));
  Gtk::RadioMenuItem *rb_medium = Gtk::manage(new Gtk::RadioMenuItem(speed_group, "Medium"));
  Gtk::RadioMenuItem *rb_fast = Gtk::manage(new Gtk::RadioMenuItem(speed_group, "Fast"));

  add_accelerator(rb_slow, '1');
  add_accelerator(rb_medium, '2');
  add_accelerator(rb_fast, '3');

  sigc::slot1<void, int> speed_slot = sigc::mem_fun(this, &AppWindow::setTickDelay);
  rb_slow->signal_activate().connect(sigc::bind(speed_slot, SLOW_TICK_DELAY));
  rb_medium->signal_activate().connect(sigc::bind(speed_slot, MEDIUM_TICK_DELAY));
  rb_fast->signal_activate().connect(sigc::bind(speed_slot, FAST_TICK_DELAY));

  m_menu_speed.items().push_back(*rb_slow);
  m_menu_speed.items().push_back(*rb_medium);
  m_menu_speed.items().push_back(*rb_fast);

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Draw Mode", m_menu_draw_mode));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Speed", m_menu_speed));

  // Pack in our widgets

  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
  m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

  // Put the viewer below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_viewer->set_size_request(300, 600);
  m_vbox.pack_start(*m_viewer);

  show_all();

  AppWindow::reset();
  setupTickTimer();
  Glib::signal_timeout().connect(sigc::mem_fun(m_viewer, &Viewer::refresh), REFRESH_DELAY);
}

AppWindow::~AppWindow() {
  delete m_viewer;
  delete m_game;
}

void AppWindow::add_accelerator(Gtk::MenuItem *it, char accelerator) {
  it->add_accelerator("activate", get_accel_group(), accelerator, (Gdk::ModifierType) 0, Gtk::ACCEL_VISIBLE);
  it->add_accelerator("activate", get_accel_group(), accelerator, Gdk::SHIFT_MASK, Gtk::ACCEL_VISIBLE);
}


bool AppWindow::on_key_press_event( GdkEventKey *ev ) {
  bool need_invalidate = false;

  if (ev->keyval == GDK_Left) {
    m_game->moveLeft();
    need_invalidate = true;
  } else if (ev->keyval == GDK_Right) {
    m_game->moveRight();
    need_invalidate = true;
  } else if (ev->keyval == GDK_Up) {
    m_game->rotateCCW();
    need_invalidate = true;
  } else if (ev->keyval == GDK_Down) {
    m_game->rotateCW();
    need_invalidate = true;
  } else if (ev->keyval == ' ') {
    m_game->drop();
    need_invalidate = true;
  } else {
    return Gtk::Window::on_key_press_event( ev );
  }
  if (need_invalidate) {
    // TODO
    //m_viewer->invalidate();
  }
  return true;
}

void AppWindow::setTickDelay(int td) {
  if (m_tick_delay == td) {
    return;
  }
  m_tick_delay = td;
  setupTickTimer();
}

void AppWindow::setupTickTimer() {
  if (!m_tick_connection.empty()) {
    m_tick_connection.disconnect();
  }
  m_tick_connection = Glib::signal_timeout().connect(sigc::mem_fun(this, &AppWindow::tick), m_tick_delay);
}

bool AppWindow::tick() {
  // TODO: Effects on row clear?
  // TODO: Display something on lose?
  int result = m_game->tick();
  if (result < 0) {
    m_viewer->setGameOver(true);
  } else if (result > 0) {
    // 1-4 rows destroyed.
  }
  return true;
}

void AppWindow::reset() {
  // TODO: Reset speed as well?
  m_viewer->resetView();
  m_menu_draw_mode.items()[1].activate();
}

void AppWindow::newgame() {
  m_game->reset();
  m_viewer->setGameOver(false);
}

