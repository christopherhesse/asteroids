#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <QuartzCore/CVDisplayLink.h>

void fatal(const char *fmt, ...);
void setup();
void keyboardInput(int, bool);
void frame(double);

GLuint ScreenFramebuffer;
GLuint ScreenRenderbuffer;
int ScreenWidth;
int ScreenHeight;

@interface View : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}

@end

@implementation View

double m_timeUnitToSeconds;

- (CVReturn)getFrameForTime:(const CVTimeStamp *)outputTime {
    // There is no autorelease pool when this method is called
    // because it will be called from a background thread.
    // It's important to create one or app can leak objects.
    
    @autoreleasepool {
        [[self openGLContext] makeCurrentContext];
        
        // We draw on a secondary thread through the display link
        // When resizing the view, -reshape is called automatically on the main
        // thread. Add a mutex around to avoid the threads accessing the context
        // simultaneously when resizing
        CGLLockContext([[self openGLContext] CGLContextObj]);
        
        frame(mach_absolute_time() * m_timeUnitToSeconds);
        
        CGLFlushDrawable([[self openGLContext] CGLContextObj]);
        CGLUnlockContext([[self openGLContext] CGLContextObj]);
    }

    return kCVReturnSuccess;
}

- (void)keyUp:(NSEvent *)event {
    keyboardInput(event.keyCode, false);
}

- (void)keyDown:(NSEvent *)event {
    keyboardInput(event.keyCode, true);
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)awakeFromNib {
    NSOpenGLPixelFormatAttribute attrs[] =
        {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAOpenGLProfile,
            NSOpenGLProfileVersion4_1Core,
            0};

    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];

    if (!pf) {
        fatal("No OpenGL pixel format");
    }

    NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];

    [self setPixelFormat:pf];
    [self setOpenGLContext:context];
    [self setWantsBestResolutionOpenGLSurface:YES];
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp *now,
                                      const CVTimeStamp *outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags *flagsOut,
                                      void *displayLinkContext) {
    CVReturn result = [(__bridge View *)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (void)reshape {
    [super reshape];
    ScreenWidth = self.bounds.size.width * [self.window.screen backingScaleFactor];
    ScreenHeight = self.bounds.size.height * [self.window.screen backingScaleFactor];
}

- (void)prepareOpenGL {
    [super prepareOpenGL];

    // Make all the OpenGL calls to setup rendering
    //  and build the necessary rendering objects
    
    // The reshape function may have changed the thread to which our OpenGL
    // context is attached before prepareOpenGL and initGL are called.  So call
    // makeCurrentContext to ensure that our OpenGL context current to this
    // thread (i.e. makeCurrentContext directs all OpenGL calls on this thread
    // to [self openGLContext])
    [[self openGLContext] makeCurrentContext];
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    ScreenFramebuffer = 0;
    
    [self reshape];
    setup();
    
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    m_timeUnitToSeconds = (double)info.numer / (info.denom * 1.0e9);

    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);

    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void *)(self));

    // Set the display link for the current renderer
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    // Activate the display link
    CVDisplayLinkStart(displayLink);

    // Register to be notified when the window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:[self window]];
}

- (void)renewGState {
    // Called whenever graphics state updated (such as window resize)

    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and drawing done by the app with other APIs)
    [[self window] disableScreenUpdatesUntilFlush];

    [super renewGState];
}

- (void)windowWillClose:(NSNotification *)notification {
    // Stop the display link when the window is closing because default
    // OpenGL render buffers will be destroyed.  If display link continues to
    // fire without renderbuffers, OpenGL draw calls will set errors.
    
    CVDisplayLinkStop(displayLink);
}

@end

int main(int argc, char *argv[]) {
    return NSApplicationMain(argc, (const char **)argv);
}
