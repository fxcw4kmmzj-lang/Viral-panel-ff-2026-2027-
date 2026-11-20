//
//  APAdvancedPanel.h
//  AdvancedPanel — Objective-C wrapper over the C ABI.
//
//  Copyright (c) 2026 Sotos. All rights reserved. See LICENSE.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, APTheme) {
    APThemeDark  = 0,
    APThemeLight = 1,
};

typedef NS_ENUM(NSInteger, APWidgetKind) {
    APWidgetKindToggle = 0,
    APWidgetKindSlider = 1,
    APWidgetKindButton = 2,
    APWidgetKindLabel  = 3,
};

typedef void (^APEventBlock)(NSString *tabID, NSString *widgetID, NSInteger kind, double value);

@interface APAdvancedPanel : NSObject

@property (nonatomic, copy, readonly) NSString *title;
@property (nonatomic, copy, readonly) NSString *version;
@property (nonatomic, assign, readonly) NSInteger tabCount;

- (nullable instancetype)initWithTitle:(NSString *)title
                               version:(NSString *)version
                                author:(NSString *)author
                                 width:(NSInteger)width
                                height:(NSInteger)height
                                 theme:(APTheme)theme;

- (BOOL)addTab:(NSString *)tabID title:(NSString *)tabTitle;
- (BOOL)removeTab:(NSString *)tabID;

- (BOOL)addToggleInTab:(NSString *)tabID
                widget:(NSString *)widgetID
                 label:(NSString *)label
               initial:(BOOL)initial;

- (BOOL)addSliderInTab:(NSString *)tabID
                widget:(NSString *)widgetID
                 label:(NSString *)label
                   min:(double)minValue
                   max:(double)maxValue
               initial:(double)initial;

- (BOOL)addButtonInTab:(NSString *)tabID
                widget:(NSString *)widgetID
                 label:(NSString *)label;

- (BOOL)addLabelInTab:(NSString *)tabID
               widget:(NSString *)widgetID
                 text:(NSString *)text;

- (BOOL)toggleValueForWidget:(NSString *)widgetID;
- (void)setToggleValue:(BOOL)value forWidget:(NSString *)widgetID;

- (double)sliderValueForWidget:(NSString *)widgetID;
- (BOOL)setSliderValue:(double)value forWidget:(NSString *)widgetID;

- (void)setEventHandler:(nullable APEventBlock)handler;

/// Simulates input coming from the host UI layer.
- (BOOL)dispatchClickInTab:(NSString *)tabID widget:(NSString *)widgetID;
- (BOOL)dispatchSliderInTab:(NSString *)tabID widget:(NSString *)widgetID value:(double)value;

/// JSON snapshot of the whole panel, useful for logging and state restore.
- (nullable NSString *)JSONRepresentation;

@end

NS_ASSUME_NONNULL_END
