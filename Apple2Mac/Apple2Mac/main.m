//
//  main.m
//  Apple2Mac
//
//  Created by Aaron Culliney on 6/21/14.
//  Copyright (c) 2014 deadc0de.org. All rights reserved.
//

#if ESSENTIAL_GL_PRACTICES_IOS
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

extern int argc;
extern const char **argv;

int main(int argc_, const char *argv_[])
{
	int retVal = 1;
    argc = argc_;
    argv = argv_;
    
#if ESSENTIAL_GL_PRACTICES_IOS
    @autoreleasepool {
        retVal = UIApplicationMain(argc, argv, nil, nil);
    }
#else
	retVal = NSApplicationMain(argc, argv);
#endif
	
    return retVal;
}
