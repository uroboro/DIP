#import <UIKit/UIKit.h>

#import "OCVImageOperator.h"

@interface DIPViewController : UIViewController <UIScrollViewDelegate, UIImagePickerControllerDelegate, UINavigationControllerDelegate>

@property (nonatomic, retain) UIScrollView *scrollView;
@property (nonatomic, retain) UIButton *actionButton;
@property (nonatomic, retain) UIButton *configButton;
@property (nonatomic, retain) UIImageView *cameraView;

@property (nonatomic, retain) UIImage *currentImage;
@property (nonatomic, retain) OCVImageOperator *imageOperatorImage;
@property (nonatomic, retain) OCVImageOperator *imageOperatorVideo;

@property (nonatomic, retain) NSMutableDictionary *options;

@end
