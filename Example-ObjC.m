//
//  main.m — Objective-C host example.
//  Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.
//

#import <Foundation/Foundation.h>
#import "APAdvancedPanel.h"

int main(int argc, const char *argv[]) {
    @autoreleasepool {
        APAdvancedPanel *panel = [[APAdvancedPanel alloc] initWithTitle:@"Advanced Panel"
                                                               version:@"1.0.0"
                                                                author:@"Sotos"
                                                                 width:1280
                                                                height:720
                                                                 theme:APThemeDark];
        if (!panel) return 1;

        [panel addTab:@"graphics" title:@"Graphics"];
        [panel addTab:@"audio" title:@"Audio"];

        [panel addToggleInTab:@"graphics" widget:@"vsync" label:@"V-Sync" initial:YES];
        [panel addSliderInTab:@"graphics" widget:@"render_scale" label:@"Render Scale"
                          min:0.5 max:2.0 initial:1.0];
        [panel addSliderInTab:@"audio" widget:@"master" label:@"Master Volume"
                          min:0.0 max:1.0 initial:0.8];
        [panel addButtonInTab:@"audio" widget:@"reset" label:@"Reset to Defaults"];

        [panel setEventHandler:^(NSString *tab, NSString *widget, NSInteger kind, double value) {
            NSLog(@"[event] tab=%@ widget=%@ kind=%ld value=%.3f", tab, widget, (long)kind, value);
        }];

        [panel dispatchClickInTab:@"graphics" widget:@"vsync"];
        [panel dispatchSliderInTab:@"audio" widget:@"master" value:0.35];

        NSLog(@"%@", [panel JSONRepresentation]);
    }
    return 0;
}
