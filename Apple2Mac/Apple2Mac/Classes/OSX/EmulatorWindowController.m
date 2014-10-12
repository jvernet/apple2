//
//  EmulatorWindowController.m
//  Apple2Mac
//
//  Created by Aaron Culliney on 9/27/14.
//  Copyright (c) 2014 deadc0de.org. All rights reserved.
//

// Based on sample code from https://developer.apple.com/library/mac/samplecode/GLEssentials/Introduction/Intro.html

#import "EmulatorWindowController.h"
#import "EmulatorFullscreenWindow.h"
#import "common.h"

#define READONLY_CHOICE_INDEX 0
#define NO_DISK_INSERTED @"(No Disk Inserted)"
#define DSK_PROPERTIES @".dsk 143360 bytes"
#define NIB_PROPERTIES @".nib 232960 bytes"
#define GZ_EXTENSION @"gz"

#define KEYCODE_CAPS_LOCK 0x39

@interface EmulatorWindowController ()

@property (nonatomic, assign) IBOutlet EmulatorGLView *view;
@property (nonatomic, retain) EmulatorFullscreenWindow *fullscreenWindow;
@property (nonatomic, retain) NSWindow *standardWindow;

@property (assign) IBOutlet NSWindow *disksWindow;
@property (assign) IBOutlet NSTextField *diskInA;
@property (assign) IBOutlet NSTextField *diskInB;
@property (assign) IBOutlet NSTextField *diskAProperties;
@property (assign) IBOutlet NSTextField *diskBProperties;
@property (assign) IBOutlet NSMatrix *diskAProtection;
@property (assign) IBOutlet NSMatrix *diskBProtection;
@property (assign) IBOutlet NSButton *chooseDiskA;
@property (assign) IBOutlet NSButton *chooseDiskB;

@property (nonatomic, copy) NSString *diskAPath;
@property (nonatomic, copy) NSString *diskBPath;

@end


@implementation EmulatorWindowController

@synthesize view = _view;
@synthesize fullscreenWindow = _fullscreenWindow;
@synthesize standardWindow = _standardWindow;
@synthesize diskAPath = _diskAPath;
@synthesize diskBPath = _diskBPath;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];

    if (self)
    {
        // Initialize to nil since it indicates app is not fullscreen
        self.fullscreenWindow = nil;
    }

    return self;
}

- (void)dealloc
{
#warning TODO FIXME ... probably should exit emulator if this gets invoked ...
    self.diskAPath = nil;
    self.diskBPath = nil;
    [super dealloc];
}

- (void)awakeFromNib
{
    [self.diskInA setStringValue:NO_DISK_INSERTED];
    [self.diskAProperties setStringValue:@""];
    [self.diskInB setStringValue:NO_DISK_INSERTED];
    [self.diskBProperties setStringValue:@""];
    
    [self.chooseDiskA setKeyEquivalent:@"\r"];
    [self.chooseDiskA setBezelStyle:NSRoundedBezelStyle];
}

- (IBAction)reboot:(id)sender
{
    [[self disksWindow] close];
    cpu65_reboot();
}

- (IBAction)toggleFullScreen:(id)sender
{
    if (self.fullscreenWindow)
    {
        [self goWindow];
    }
    else
    {
        [self goFullscreen];
    }
}

- (IBAction)diskAProtectionChanged:(id)sender
{
    if ([[[self diskInA] stringValue] isEqualToString:NO_DISK_INSERTED])
    {
        return;
    }
    // HACK NOTE : dispatch so that state of outlet property is set properly
    dispatch_async(dispatch_get_main_queue(), ^{
        NSButtonCell *readOnlyChoice = [[[self diskAProtection] cells] firstObject];
        NSString *path = [self diskAPath];
        [self _insertDisketteInDrive:0 path:path type:[self _extensionForPath:path] readOnly:([readOnlyChoice state] == NSOnState)];
    });
}

- (IBAction)diskBProtectionChanged:(id)sender
{
    if ([[[self diskInB] stringValue] isEqualToString:NO_DISK_INSERTED])
    {
        return;
    }
    // HACK NOTE : dispatch so that state of outlet property is set properly
    dispatch_async(dispatch_get_main_queue(), ^{
        NSButtonCell *readOnlyChoice = [[[self diskBProtection] cells] firstObject];
        NSString *path = [self diskBPath];
        [self _insertDisketteInDrive:1 path:path type:[self _extensionForPath:path] readOnly:([readOnlyChoice state] == NSOnState)];
    });
}

- (BOOL)_insertDisketteInDrive:(int)drive path:(NSString *)path type:(NSString *)type readOnly:(BOOL)readOnly
{
    c_eject_6(drive);
    
    const char *errMsg = c_new_diskette_6(drive, [path UTF8String], readOnly);
    if (errMsg)
    {
        NSAlert *alert = [NSAlert alertWithError:[NSError errorWithDomain:[NSString stringWithUTF8String:errMsg] code:-1 userInfo:nil]];
        [alert beginSheetModalForWindow:[self disksWindow] completionHandler:nil];
        if (!drive)
        {
            [[self diskInA] setStringValue:NO_DISK_INSERTED];
            [[self diskAProperties] setStringValue:@""];
        }
        else
        {
            [[self diskInB] setStringValue:NO_DISK_INSERTED];
            [[self diskBProperties] setStringValue:@""];
        }
        
        return NO;
    }

    NSString *imageName = [[path pathComponents] lastObject];

    if (!drive)
    {
        self.diskAPath = path;
        [[self diskInA] setStringValue:imageName];
    }
    else
    {
        self.diskBPath = path;
        [[self diskInB] setStringValue:imageName];
    }
    
    if ([type isEqualToString:@"dsk"] || [type isEqualToString:@"do"] || [type isEqualToString:@"po"])
    {
        if (!drive)
        {
            [[self diskAProperties] setStringValue:DSK_PROPERTIES];
        }
        else
        {
            [[self diskBProperties] setStringValue:DSK_PROPERTIES];
        }
    }
    else
    {
        if (!drive)
        {
            [[self diskAProperties] setStringValue:NIB_PROPERTIES];
        }
        else
        {
            [[self diskBProperties] setStringValue:NIB_PROPERTIES];
        }
    }

    return YES;
}

- (NSString *)_extensionForPath:(NSString *)path
{
    NSString *extension0 = [path pathExtension];
    NSString *extension1 = [[path stringByDeletingPathExtension] pathExtension];
    if ([extension0 isEqualToString:GZ_EXTENSION])
    {
        extension0 = extension1;
    }
    return extension0;
}

- (void)_chooseDisk:(int)drive readOnly:(BOOL)readOnly
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setTitle:@"Choose a disk image"];
#warning FIXME TODO : installation default should be what it bundled, but should not always choose this ...
    NSURL *url = [[NSBundle mainBundle] URLForResource:@"blank" withExtension:@"dsk.gz"];
    url = [url URLByDeletingLastPathComponent];
    [openPanel setDirectoryURL:url];
    [openPanel setShowsResizeIndicator:YES];
    [openPanel setShowsHiddenFiles:NO];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setCanCreateDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    
    // NOTE : Doesn't appear to be a way to specify ".dsk.gz" ... so we may inadvertently allow files of ".foo.gz" here
    NSSet *fileTypes = [NSSet setWithObjects:@"dsk", @"nib", @"do", @"po", GZ_EXTENSION, nil];
    [openPanel setAllowedFileTypes:[fileTypes allObjects]];
    [openPanel beginSheetModalForWindow:[self disksWindow] completionHandler:^(NSInteger result) {
        if (result == NSOKButton)
        {
            NSURL *selection = [[openPanel URLs] firstObject];
            NSString *path = [[selection path] stringByResolvingSymlinksInPath];
            NSString *extension = [self _extensionForPath:path];
            
            if (![fileTypes containsObject:extension])
            {
                NSAlert *alert = [NSAlert alertWithError:[NSError errorWithDomain:@"Disk image must have file extension of .dsk, .do, .po, or .nib only" code:-1 userInfo:nil]];
                [alert beginSheetModalForWindow:[self disksWindow] completionHandler:nil];
                return;
            }
            
            [self _insertDisketteInDrive:drive path:path type:extension readOnly:readOnly];
        }
    }];
}

- (IBAction)chooseDriveA:(id)sender
{
    NSButtonCell *readOnlyChoice = [[[self diskAProtection] cells] firstObject];
    [self _chooseDisk:0 readOnly:([readOnlyChoice state] == NSOnState)];
}

- (IBAction)chooseDriveB:(id)sender
{
    NSButtonCell *readOnlyChoice = [[[self diskBProtection] cells] firstObject];
    [self _chooseDisk:1 readOnly:([readOnlyChoice state] == NSOnState)];
}

- (void)goFullscreen
{
    // If app is already fullscreen...
    if (self.fullscreenWindow)
    {
        //...don't do anything
        return;
    }

    // Allocate a new fullscreen window
    self.fullscreenWindow = [[[EmulatorFullscreenWindow alloc] init] autorelease];

    // Resize the view to screensize
    NSRect viewRect = [self.fullscreenWindow frame];

    // Set the view to the size of the fullscreen window
    [self.view setFrameSize: viewRect.size];

    // Set the view in the fullscreen window
    [self.fullscreenWindow setContentView:self.view];

    self.standardWindow = [self window];

    // Hide non-fullscreen window so it doesn't show up when switching out
    // of this app (i.e. with CMD-TAB)
    [self.standardWindow orderOut:self];

    // Set controller to the fullscreen window so that all input will go to
    // this controller (self)
    [self setWindow:self.fullscreenWindow];

    // Show the window and make it the key window for input
    [self.fullscreenWindow makeKeyAndOrderFront:self];
}

- (void)goWindow
{
    // If controller doesn't have a full screen window...
    if (self.fullscreenWindow == nil)
    {
        //...app is already windowed so don't do anything
        return;
    }

    // Get the rectangle of the original window
    NSRect viewRect = [self.standardWindow frame];
    
    // Set the view rect to the new size
    [self.view setFrame:viewRect];

    // Set controller to the standard window so that all input will go to
    // this controller (self)
    [self setWindow:self.standardWindow];

    // Set the content of the orginal window to the view
    [[self window] setContentView:self.view];

    // Show the window and make it the key window for input
    [[self window] makeKeyAndOrderFront:self];

    // Release/nilify fullscreenWindow
    self.fullscreenWindow = nil;
}

- (void)flagsChanged:(NSEvent*)event
{
    static BOOL modified_caps_lock = NO;
    if ([event keyCode] == KEYCODE_CAPS_LOCK)
    {
        NSUInteger flags = [event modifierFlags];
        if (flags & NSAlphaShiftKeyMask)
        {
            modified_caps_lock = YES;
            caps_lock = true;
        }
        else
        {
            caps_lock = false;
        }
    }
}

- (void)keyUp:(NSEvent *)event
{
    unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];
    
    c_keys_handle_input((int)c, 0, 1);
    
    // Allow other character to be handled (or not and beep)
    //[super keyDown:event];
}

- (void)keyDown:(NSEvent *)event
{
    unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];

    c_keys_handle_input((int)c, 1, 1);

    // Allow other character to be handled (or not and beep)
    //[super keyDown:event];
}

@end
