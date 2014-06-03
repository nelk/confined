#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <gtkmm.h>
#include "viewer.hpp"

class Viewer;

class AppWindow : public Gtk::Window {
public:
  AppWindow();
  ~AppWindow();

  void redraw_label(int mode, double fov, double near, double far);

protected:

private:
  void add_accelerator(Gtk::MenuItem *it, char accelerator);

  void reset_view();

  // A "vertical box" which holds everything in our window
  Gtk::VBox m_vbox;

  // The menubar, with all the menus at the top of the window
  Gtk::MenuBar m_menubar;
  // Each menu itself
  Gtk::Menu m_menu_app;
  Gtk::Menu m_menu_mode;

  Gtk::Label m_main_label;

  // The main OpenGL area
  Viewer* m_viewer;
};

#endif
