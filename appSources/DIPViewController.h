#import <UIKit/UIKit.h>

#import "OCVImageOperator.h"

@interface DIPViewController : UIViewController <UIImagePickerControllerDelegate, UINavigationControllerDelegate>

@property (nonatomic, retain) UIScrollView *scrollView;
@property (nonatomic, retain) UIButton *actionButton;
@property (nonatomic, retain) UIButton *configButton;

@property (nonatomic, retain) UIImage *currentImage;
@property (nonatomic, retain) OCVImageOperator *imageOperator;

@end

