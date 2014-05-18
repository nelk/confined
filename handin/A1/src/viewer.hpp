
#ifndef CS488_VIEWER_HPP
#define CS488_VIEWER_HPP

#include <gtkmm.h>
#include <gtkglmm.h>

#include "game.hpp"
#include "algebra.hpp"

// The "main" OpenGL widget
class Viewer : public Gtk::GL::DrawingArea {
  public:
    enum ViewMode {
      WIREFRAME, FACES, MULTICOLOUR
    };

    Viewer(Game *game);
    virtual ~Viewer();

    // A useful function that forces this widget to rerender. If you
    // want to render a new frame, do not call on_expose_event
    // directly. Instead call this, which will cause an on_expose_event
    // call when the time is right.
    void invalidate();
    bool refresh(); // Viewing animation.
    void resetView();

    void setViewMode(ViewMode vm) {
      m_view_mode = vm;
      invalidate();
    }

    void setGameOver(bool go) {
      m_game_over = go;
    }

  protected:

    // Events we implement
    // Note that we could use gtkmm's "signals and slots" mechanism
    // instead, but for many classes there's a convenient member
    // function one just needs to define that'll be called with the
    // event.

    // Called when GL is first initialized
    virtual void on_realize();
    // Called when our window needs to be redrawn
    virtual bool on_expose_event(GdkEventExpose* event);
    // Called when the window is resized
    virtual bool on_configure_event(GdkEventConfigure* event);
    // Called when a mouse button is pressed
    virtual bool on_button_press_event(GdkEventButton* event);
    // Called when a mouse button is released
    virtual bool on_button_release_event(GdkEventButton* event);
    // Called when the mouse moves
    virtual bool on_motion_notify_event(GdkEventMotion* event);

  private:
    void drawCube(bool differentColours = false, bool randomColours = false);
    void setColourForId(int id);

    ViewMode m_view_mode;
    Game *m_game;
    Vector3D m_rot, m_rotv;
    double m_scale;
    bool m_currently_scaling;
    guint m_mouse_button;
    gdouble m_last_mouse_x;
    gdouble m_last_delta_x;
    bool m_game_over;
};

#endif
