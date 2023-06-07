// NOTE: To run, it is recommended not to be in Compiz or Beryl, they have shown some instability.

#include <wx/wx.h>
#include <wx/glcanvas.h>

#ifdef __WXMAC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#ifndef WIN32
#include <unistd.h> // FIXME: This work/necessary in Windows?
                    //Not necessary, but if it was, it needs to be replaced by process.h AND io.h
#endif


#ifdef WIN32
// The following function is taken nearly verbatim from
// http://www.halcyon.com/~ast/dload/guicon.htm
void RedirectIOToConsole()
{
  int hConHandle;
  long lStdHandle;
  FILE *fp;

  // redirect unbuffered STDOUT to the console
  lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  *stdout = *fp;
  setvbuf( stdout, NULL, _IONBF, 0 );

  // redirect unbuffered STDIN to the console
  lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "r" );
  *stdin = *fp;
  setvbuf( stdin, NULL, _IONBF, 0 );

  // redirect unbuffered STDERR to the console
  lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
  hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
  fp = _fdopen( hConHandle, "w" );
  *stderr = *fp;
  setvbuf( stderr, NULL, _IONBF, 0 );

  // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
  // point to console as well
  std::ios::sync_with_stdio();
}

class UseConsole
{
public:
  UseConsole();
  ~UseConsole();
  static void AttachToConsole();
  static void DetachFromConsole();
private:
  static bool _good;
};

bool UseConsole::_good = false;
UseConsole::UseConsole() {
}

UseConsole::~UseConsole() {
}

void UseConsole::AttachToConsole() {
  if (!_good) {
    _good = AttachConsole(ATTACH_PARENT_PROCESS);
    if (_good)
      RedirectIOToConsole();
  }
}

void UseConsole::DetachFromConsole() {
  if (_good)
    FreeConsole();
}

#endif /* WIN32 */

class wxGLCanvasSubClass: public wxGLCanvas {
        void Render();
public:
    wxGLCanvasSubClass(wxFrame* parent);
    void Paintit(wxPaintEvent& event);
protected:
    DECLARE_EVENT_TABLE()
private:
    // OpenGL context (to be initialized when the window is shown)
    wxGLContext *_glContext;
};

BEGIN_EVENT_TABLE(wxGLCanvasSubClass, wxGLCanvas)
    EVT_PAINT    (wxGLCanvasSubClass::Paintit)
END_EVENT_TABLE()

// Double buffering (Fix B70)
static int wx_gl_attribs[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 24, 0};

wxGLCanvasSubClass::wxGLCanvasSubClass(wxFrame *parent)
: wxGLCanvas(parent, wxID_ANY, wx_gl_attribs, wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"))
{
    int argc = 1;
    char* argv[1] = { wxString((wxTheApp->argv)[0]).char_str() };
    // OpenGL context initialization
    _glContext = new wxGLContext(this);

/*
NOTE: this example uses GLUT in order to have a free teapot model
to display, to show 3D capabilities. GLUT, however, seems to cause problems
on some systems. If you meet problems, first try commenting out glutInit(),
then try comeenting out all glut code
*/
    glutInit(&argc, argv);
}


void wxGLCanvasSubClass::Paintit(wxPaintEvent& WXUNUSED(event)){
    Render();
}

void wxGLCanvasSubClass::Render()
{
    SetCurrent(*_glContext);
    wxPaintDC(this);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, (GLint)GetSize().x, (GLint)GetSize().y);

    glBegin(GL_POLYGON);
        glColor3f(1.0, 1.0, 1.0);
        glVertex2f(-0.5, -0.5);
        glVertex2f(-0.5, 0.5);
        glVertex2f(0.5, 0.5);
        glVertex2f(0.5, -0.5);
        glColor3f(0.4, 0.5, 0.4);
        glVertex2f(0.0, -0.8);
    glEnd();

    glBegin(GL_POLYGON);
        glColor3f(1.0, 0.0, 0.0);
        glVertex2f(0.1, 0.1);
        glVertex2f(-0.1, 0.1);
        glVertex2f(-0.1, -0.1);
        glVertex2f(0.1, -0.1);
    glEnd();

// using a little of glut
    glColor4f(0,0,1,1);
    glutWireTeapot(0.4);

    glLoadIdentity();
    glColor4f(2,0,1,1);
    glutWireTeapot(0.6);
// done using glut

    glFlush();
    SwapBuffers();
}

class MyApp: public wxApp
{
    virtual bool OnInit();
    wxGLCanvas * MyGLCanvas;
#ifdef WIN32
    virtual int OnExit() ;
#endif
};

 
IMPLEMENT_APP_CONSOLE(MyApp) ;
 
  
bool MyApp::OnInit()
{
#ifdef WIN32
    UseConsole::AttachToConsole();
    std::cout << std::endl;
#endif

    wxFrame *frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Hello GL World"), wxPoint(50,50), wxSize(200,200));
    wxGLCanvasSubClass* canvas = new wxGLCanvasSubClass(frame);
 
    frame->Show(TRUE);
    return TRUE;
}

#ifdef WIN32
int MyApp::OnExit() {
  UseConsole::DetachFromConsole();
  return wxApp::OnExit();
}
#endif
