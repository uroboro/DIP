#ifndef COMMON_H
#define COMMON_H

#include <objc/runtime.h>
#import <UIKit/UIAlertView.h>

#define UIAlert(t, m) dispatch_async(dispatch_get_main_queue(), ^{ [[[[objc_getClass("UIAlertView") alloc] initWithTitle:(t) message:(m) delegate:0 cancelButtonTitle:@"OK" otherButtonTitles:0] autorelease] show]; })

#endif