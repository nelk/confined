
#include "paintwindow.hpp"
#include <gdkmm/color.h>
#include <gtkmm/radiomenuitem.h>

PaintWindow::PaintWindow() {
  set_title("488 Paint");

  // A utility class for constructing things that go into menus, which
  // we'll set up next.
  using Gtk::Menu_Helpers::MenuElem;

  // Set up the application menu
  // The slot we use here just causes PaintWindow::hide() on this,
  // which shuts down the application.
  m_menu_app.items().push_back(MenuElem("_Clear", Gtk::AccelKey("C"),
        sigc::mem_fun(*this, &PaintWindow::clear)));
  m_menu_app.items().push_back(MenuElem("_Quit", Gtk::AccelKey("Q"),
        sigc::mem_fun(*this, &PaintWindow::hide)));

  // Set up the tools menu

  // We're going to be connecting a bunch of menu entries to the same
  // function. So, we put a slot corresponding to that function in
  // mode_slot.
  //
  // The type shows that this slot returns void (nothing, and takes
  // one argument, a PaintCanvas::Mode.
  sigc::slot1<void, PaintCanvas::Mode> mode_slot = 
    sigc::mem_fun(m_canvas, &PaintCanvas::set_mode);

  // Now we set up the actual tools. SigC::bind takes a slot and makes
  // a new slot with one fewer parameter than the one passed to it,
  // and "binds in" the constant value passed.
  //
  // In our case we take the reference to PaintCanvas::set_mode we
  // declared above, and bind in the appropriate mode, making a slot
  // that calls set_mode with the given mode (line/oval/rectangle).

  Gtk::RadioMenuItem::Group group;
  Gtk::RadioMenuItem *rb_line = Gtk::manage(new Gtk::RadioMenuItem(group, "Line"));
  Gtk::RadioMenuItem *rb_oval = Gtk::manage(new Gtk::RadioMenuItem(group, "Oval"));
  Gtk::RadioMenuItem *rb_rectangle = Gtk::manage( new Gtk::RadioMenuItem(group, "Rectangle"));

  rb_line->signal_select().connect(sigc::bind( mode_slot, PaintCanvas::DRAW_LINE));
  rb_oval->signal_select().connect(sigc::bind( mode_slot, PaintCanvas::DRAW_OVAL));
  rb_rectangle->signal_select().connect(sigc::bind( mode_slot, PaintCanvas::DRAW_RECTANGLE));

  m_menu_tools.items().push_back(*rb_line);
  m_menu_tools.items().push_back(*rb_oval);
  m_menu_tools.items().push_back(*rb_rectangle);

  // Colour menu.
  sigc::slot1<void, Gdk::Color> colour_slot =
    sigc::mem_fun(m_canvas, &PaintCanvas::set_colour);

  Gdk::Color black, red, green, blue;
  black.set("black");
  red.set("red");
  green.set("green");
  blue.set("blue");

  m_menu_colours.items().push_back( MenuElem("_Black", 
        sigc::bind( colour_slot, black ) ) );
  m_menu_colours.items().push_back( MenuElem("_Red", 
        sigc::bind( colour_slot, red ) ) );
  m_menu_colours.items().push_back( MenuElem("_Green", 
        sigc::bind( colour_slot, green ) ) );
  m_menu_colours.items().push_back( MenuElem("_Blue", 
        sigc::bind( colour_slot, blue ) ) );


  // Set up the help menu
  m_menu_help.items().push_back(MenuElem("_Line Help",
        sigc::mem_fun(*this, &PaintWindow::help_line)));
  m_menu_help.items().push_back(MenuElem("_Oval Help",
        sigc::mem_fun(*this, &PaintWindow::help_oval)));
  m_menu_help.items().push_back(MenuElem("_Rectangle Help",
        sigc::mem_fun(*this, &PaintWindow::help_rectangle)));

  // Set up the menu bar
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Application", m_menu_app));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Tools", m_menu_tools));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Colours", m_menu_colours));
  m_menubar.items().push_back(Gtk::Menu_Helpers::MenuElem("_Help", m_menu_help));
  m_menubar.items().back().set_right_justified(true);

  // Pack in our widgets

  // First add the vertical box as our single "top" widget
  add(m_vbox);

  // Put the menubar on the top, and make it as small as possible
  m_vbox.pack_start(m_menubar, Gtk::PACK_SHRINK);

  // Put the canvas below the menubar. pack_start "grows" the widget
  // by default, so it'll take up the rest of the window.
  m_canvas.set_size_request(300, 300);
  m_vbox.pack_start(m_canvas);

  quit_button.set_label("Quit");
  m_vbox.pack_start(quit_button);
  quit_button.signal_clicked().connect(sigc::mem_fun(*this, &PaintWindow::hide));

  show_all();
}

void PaintWindow::help_dialog(const char *message) {
  Gtk::MessageDialog dialog(*this, message, true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
  dialog.run();
}

void PaintWindow::help_line() {
  const char* message =
    "Drawing a Line\n"
    "\n"
    "To draw a line, press the left mouse button to mark the beginning of the line.  Drag the mouse to the end of the line and release the button.";
  help_dialog(message);
}

void PaintWindow::help_oval() {
  const char* message =
    "Drawing an Oval\n"
    "\n"
    "To draw an oval, press the left mouse button to mark one point.  Drag the mouse to the to form a rectangle, and release the button. An oval will be drawn within the rectangle.";
  help_dialog(message);
}

void PaintWindow::help_rectangle() {
  const char* message =
    "Drawing a Rectangle\n"
    "\n"
    "To draw a rectangle, press the left mouse button to mark one point.  Drag the mouse to the to form a rectangle, and release the button.";
  help_dialog(message);
}


void PaintWindow::clear() {
  m_canvas.clear();
}

