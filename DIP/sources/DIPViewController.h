#import <UIKit/UIKit.h>

@interface DIPViewController : UIViewController <UIImagePickerControllerDelegate, UINavigationControllerDelegate>

@property (nonatomic, retain) UIScrollView *scrollView;
@property (nonatomic, retain) UIButton *actionButton;
@property (nonatomic, retain) UIButton *configButton;

@property (nonatomic, retain) UIImage *currentImage;

@end

