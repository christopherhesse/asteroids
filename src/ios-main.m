#import <UIKit/UIKit.h>
#import <OpenGLES/ES3/gl.h>
#import <GLKit/GLKit.h>

void fatal(const char *fmt,...);
void setup();
void touchInput(float, float);
void frame(double);

GLuint ScreenFramebuffer;
GLuint ScreenRenderbuffer;
int ScreenWidth;
int ScreenHeight;

@interface View : UIView {
}
@end

@implementation View

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint point = [touch locationInView:nil];
        touchInput(point.x, point.y);
    }
}

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

@end

@interface ViewController : UIViewController
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view = [[View alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.view.layer;
    eaglLayer.opaque = YES;
    self.view.contentScaleFactor = [[UIScreen mainScreen] scale];
    
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES3;
    EAGLContext *context = [[EAGLContext alloc] initWithAPI:api];
    if (!context) {
        fatal("Failed to initialize context");
    }
    
    if (![EAGLContext setCurrentContext:context]) {
        fatal("failed to set current context");
    }
    
    CGSize size = [UIScreen mainScreen].bounds.size;
    ScreenWidth = size.width * self.view.contentScaleFactor;
    ScreenHeight = size.height * self.view.contentScaleFactor;
    
    glGenFramebuffers(1, &ScreenFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ScreenFramebuffer);
    
    GLuint depthRenderbuffer;
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, ScreenWidth, ScreenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    glGenRenderbuffers(1, &ScreenRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, ScreenRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ScreenRenderbuffer);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fatal("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
    
    setup();
    
    CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)render:(CADisplayLink *)displayLink {
    frame(displayLink.timestamp);
    // this needs to be set so that IOS can draw the correct buffer to the screen
    glBindRenderbuffer(GL_RENDERBUFFER, ScreenRenderbuffer);
    [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER];
}

@end

@interface AppDelegate : UIResponder <UIApplicationDelegate>

@property (strong, nonatomic) UIWindow *window;

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [UIWindow new];
    self.window.frame = [[UIScreen mainScreen] bounds];
    self.window.rootViewController = [ViewController new];
    [self.window makeKeyAndVisible];
    return YES;
}

@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
