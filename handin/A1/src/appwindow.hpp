#ifndef APPWINDOW_HPP
#define APPWINDOW_HPP

#include <gtkmm.h>
#include "viewer.hpp"
#include "game.hpp"

class AppWindow : public Gtk::Window {
  public:
    AppWindow();
    ~AppWindow();

    bool tick();
    void reset();
    void newgame();
    void setupTickTimer();

  protected:
    virtual bool on_key_press_event( GdkEventKey *ev );

  private:
    void add_accelerator(Gtk::MenuItem *it, char accelerator);

    void setTickDelay(int td);

    // A "vertical box" which holds everything in our window
    Gtk::VBox m_vbox;

    // The menubar, with all the menus at the top of the window
    Gtk::MenuBar m_menubar;
    // Each menu itself
    Gtk::Menu m_menu_app;
    Gtk::Menu m_menu_draw_mode;
    Gtk::Menu m_menu_speed;
    Gtk::Menu m_menu_extras;

    // The main OpenGL area
    Game *m_game;
    Viewer *m_viewer;
    int m_tick_delay;
    sigc::connection m_tick_connection;
};

#endif
