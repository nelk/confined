#ifndef PAINTWINDOW_HPP
#define PAINTWINDOW_HPP

#include <gtkmm.h>

#include "paintcanvas.hpp"

class PaintWindow : public Gtk::Window {
public:
  PaintWindow();
  virtual ~PaintWindow() {}

  // Help dialogs.
  void help_line();
  void help_oval();
  void help_rectangle();

  void clear();

protected:
  void help_dialog(const char *message);

private:
  // A "vertical box" which holds everything in our window
  Gtk::VBox m_vbox;

  // The menubar, with all the menus at the top of the window
  Gtk::MenuBar m_menubar;
  // Each menu itself
  Gtk::Menu m_menu_app;
  Gtk::Menu m_menu_tools;
  Gtk::Menu m_menu_colours;
  Gtk::Menu m_menu_help;
  // Quit Button
  Gtk::Button quit_button;

  // The canvas onto which the user draws.
  PaintCanvas m_canvas;
};

#endif
