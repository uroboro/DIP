#import <UIKit/UIView.h>
#import <UIKit/UIImage.h>

@interface OCVImageOperator : NSObject {
	dispatch_source_t _source;
	void *_handle;
}

@property (nonatomic, assign) UIView *view;
- (id)initWithView:(UIView *)view;

@property (nonatomic, retain) UIImage *image;
- (UIImage *)operateImage:(UIImage *)image;

@end
